/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <map>
#include <ranges>
#include <set>

#include <TODO_qtils/bytes_std_hash.hpp>
#include <TODO_qtils/cxx23/ranges/contains.hpp>
#include <qtils/append.hpp>

#include <jam/bandersnatch.hpp>
#include <jam/ed25519.hpp>
#include <test-vectors/common.hpp>
#include <test-vectors/disputes/types.hpp>

namespace jam::disputes {
  namespace types = test_vectors_disputes;

  auto asSet(auto &&r) {
    return std::set(r.begin(), r.end());
  }

  auto asVec(auto &&r) {
    return std::vector(r.begin(), r.end());
  }

  template <typename M>
  struct MultimapGroups {
    using I = typename M::const_iterator;
    const M &m;

    struct It {
      const M &m;
      I begin;
      I end;

      auto makeEnd() const {
        return begin == m.end() ? m.end() : m.upper_bound(begin->first);
      }
      It(const M &m, I begin) : m{m}, begin{begin}, end{makeEnd()} {}
      bool operator==(const It &r) const {
        return begin == r.begin;
      }
      It &operator++() {
        begin = end;
        end = makeEnd();
        return *this;
      }
      auto operator*() const {
        return std::make_pair(
            begin->first,
            std::ranges::subrange(begin, end) | std::views::values);
      }
    };

    auto begin() const {
      return It{m, m.begin()};
    }
    auto end() const {
      return It{m, m.end()};
    }
  };
  template <typename M>
  MultimapGroups(const M &) -> MultimapGroups<M>;

  inline bool ed25519_verify(const jam::ed25519::Signature &sig,
                             const jam::ed25519::Public &pub,
                             qtils::BytesIn X,
                             const types::WorkReportHash &work_report) {
    qtils::Bytes payload;
    qtils::append(payload, X);
    qtils::append(payload, work_report);
    return jam::ed25519::verify(sig, payload, pub);
  };

  // [GP 0.4.5 I.4.5]
  // $jam_valid - Ed25519 Judgments for valid work-reports.
  constexpr qtils::BytesN<9> kJamValid{
      'j', 'a', 'm', '_', 'v', 'a', 'l', 'i', 'd'};

  // [GP 0.4.5 I.4.5]
  // $jam_invalid - Ed25519 Judgments for invalid work-reports.
  constexpr qtils::BytesN<11> kJamInvalid{
      'j', 'a', 'm', '_', 'i', 'n', 'v', 'a', 'l', 'i', 'd'};

  // [GP 0.4.5 I.4.5]
  // $jam_guarantee - Ed25519 Guarantee statements.
  constexpr qtils::BytesN<13> kJamGuarantee{
      'j', 'a', 'm', '_', 'g', 'u', 'a', 'r', 'a', 'n', 't', 'e', 'e'};

  /// Given state and input, derive next state and output.
  inline std::pair<types::State, types::Output> transition(
      const types::Config &config,
      const types::State &state,
      const types::Input &input) {
    using Error = types::ErrorCode;
    const auto error = [&](Error error) {
      return std::make_pair(state, types::Output{error});
    };

    // [GP 0.4.5 10.1]
    // ψ ≡ (ψg, ψb, ψw, ψo)

    // ψ - the state of dispute
    auto &dispute_records = state.psi;

    // [GP 0.4.5 10 97]
    // ψg - set of work-reports which were judged as correct
    const auto &good_set = dispute_records.psi_g;

    // ψb - set of work-reports which were judged as incorrect
    const auto &bad_set = dispute_records.psi_b;

    // ψw - set of work-reports which were appeared impossible to judge
    const auto &wonky_set = dispute_records.psi_w;

    // ψo - a set of Ed25519 keys representing validators which were found to
    // have misjudged a work-report
    const auto &punish_set = dispute_records.psi_o;

    // [GP 0.4.5 10.2]
    // ED ≡ (v, c, f)

    // ED - the disputes extrinsic
    const auto &ext = input.disputes;

    // v - one or more verdicts as a compilation of judgments coming from 2N/3+1
    // of either the active validator set or the previous epoch’s validator set,
    // i.e. the Ed25519 keys of κ or λ
    const auto &verdicts = ext.verdicts;

    // c - set of proofs of one or more validator's misbehavior by guaranteeing
    // a work-report found to be invalid. Offenders associated with the
    // guarantee of the report
    const auto &culprits = ext.culprits;

    // f - set of proofs of one or more validators misbehavior by signing a
    // judgment found to be a contradiction to a work-report’s validity.
    // Signature related judgment offenders.
    const auto &faults = ext.faults;

    types::EpochIndex current_epoch = state.tau / config.epoch_length;

    // к - kappa, aka validator set of current epoch
    const auto &current_epoch_validator_set = state.kappa;

    types::EpochIndex previous_epoch =
        current_epoch ? current_epoch - 1
                      : 0;  // For using copy of epoch 0 as of previous one

    // λ - lambda, aka validator set of previous epoch
    const auto &previous_epoch_validator_set = state.lambda;

    // Verdicts for registration
    std::vector<types::DisputeVerdict> verdicts_registry;
    std::unordered_multimap<
        types::WorkReportHash,
        std::reference_wrapper<const types::DisputeJudgement>,
        qtils::BytesStdHash>
        judgements_registry;

    // Check verdicts.
    // The signatures of all judgments must be valid in terms of one of the two
    // allowed validator key-sets, identified by the verdict’s second term which
    // must be either the epoch index of the prior state or one less
    {
      std::optional<types::WorkReportHash> prev_work_report{};
      for (const auto &verdict : verdicts) {
        const auto &work_report = verdict.target;
        const auto &epoch = verdict.age;
        const auto &judgements = verdict.votes.v;

        if (epoch != current_epoch and epoch != previous_epoch) {
          return error(Error::bad_judgement_age);
        }

        // Verdicts must be ordered by report hash.
        // There may be no duplicate report hashes within the extrinsic,
        // nor amongst any past reported hashes
        // [GP 0.4.5 10.2 (103)]
        if (prev_work_report >= work_report) {
          return error(Error::verdicts_not_sorted_unique);
        }
        prev_work_report = work_report;

        if (judgements.size() < config.validators_super_majority) {
          return error(Error::not_enough_culprits);
        }

        const auto &validators_set = epoch == current_epoch
                                       ? current_epoch_validator_set.v
                                       : previous_epoch_validator_set.v;

        std::optional<types::U16> prev_validator_index{};
        for (const auto &judgement : judgements) {
          const auto vote = judgement.vote;
          const auto validator_index = judgement.index;
          const auto &validator_signature = judgement.signature;

          // The judgments of all verdicts must be ordered by validator index,
          // and there may be no duplicates
          // [GP 0.4.5 10.2 (106)]
          if (prev_validator_index >= validator_index) {
            return error(Error::judgements_not_sorted_unique);
          }
          prev_validator_index = validator_index;

          if (validator_index >= validators_set.size()) {
            return error(Error::bad_validator_index);
          }

          // [GP 0.4.5 10.2 (99)]
          // Ensure signature is valid
          const auto &validator_public =
              validators_set[validator_index].ed25519;
          auto is_valid_signature =
              ed25519_verify(validator_signature,
                             validator_public,
                             vote ? qtils::BytesIn{kJamValid} : kJamInvalid,
                             work_report);
          if (not is_valid_signature) {
            return error(Error::bad_signature);
          }
        }

        // [GP 0.4.5 10.2 (105)]
        // There may be no duplicate report hashes within the extrinsic, nor
        // amongst any past reported hashes
        auto in_good = qtils::cxx23::ranges::contains(good_set, work_report);
        auto in_bad = qtils::cxx23::ranges::contains(bad_set, work_report);
        auto in_wonky = qtils::cxx23::ranges::contains(wonky_set, work_report);
        if (not in_bad and not in_good and not in_wonky) {
          verdicts_registry.push_back(verdict);
          for (const auto &judgement : judgements) {
            judgements_registry.emplace(work_report, std::ref(judgement));
          }
        }
      }
    }

    std::multimap<types::WorkReportHash,
                  std::reference_wrapper<const types::DisputeCulpritProof>,
                  std::less<>
                  // std::equal_to<>
                  //  hash_range<types::WorkReportHash>
                  >
        culprits_registry;

    // Check culprits
    {
      std::optional<types::Ed25519Key> prev_validator_key{};
      for (const auto &culprit : culprits) {
        const auto &work_report = culprit.target;
        const auto &validator_key = culprit.key;
        const auto &validator_signature = culprit.signature;

        // [GP 0.4.5 10.2 (101)/3]
        // Ensure signature is valid
        const auto &validator_public = validator_key;
        auto is_valid_signature = ed25519_verify(
            validator_signature, validator_public, kJamGuarantee, work_report);
        if (not is_valid_signature) {
          return error(Error::bad_signature);
        }

        // [GP 0.4.5 10.2 (101)/1]
        // Ensure if work-report isn't in a bad yet
        if (qtils::cxx23::ranges::contains(bad_set, work_report)) {
          return error(Error::already_judged);
        }

        // [GP 0.4.5 10.2 (101)/2]
        // Ensure validator from the set of current epoch
        const auto &validators_set = current_epoch_validator_set.v;
        if (not qtils::cxx23::ranges::contains_if(
                validators_set, [&](const auto &val) {
                  return val.ed25519 == validator_key;
                })) {
          return error(Error::bad_validator_index);  // TODO check error type
        }

        // Offender signatures must be ordered by the validator’s Ed25519 key.
        // There may be no duplicate report hashes within the extrinsic, nor
        // amongst any past reported hashes.
        // [GP 0.4.5 10.2 (104)]
        if (prev_validator_key >= validator_key) {
          return error(Error::culprits_not_sorted_unique);
        }
        prev_validator_key = validator_key;

        // Not report keys which are already in the punish-set
        if (qtils::cxx23::ranges::contains(punish_set, validator_key)) {
          return error(Error::offender_already_reported);
        }

        culprits_registry.emplace(work_report, culprit);
      }
    }

    std::multimap<types::WorkReportHash,
                  std::reference_wrapper<const types::DisputeFaultProof>,
                  std::less<>
                  // hash_range<types::WorkReportHash>
                  >
        faults_registry;

    // Check faults
    {
      std::optional<types::Ed25519Key> prev_validator_key{};
      for (const auto &fault : faults) {
        const auto &work_report = fault.target;
        const auto vote = fault.vote;
        const auto &validator_key = fault.key;
        const auto &validator_signature = fault.signature;

        // Ensure if work-report isn't in a wonky set yet
        if (qtils::cxx23::ranges::contains(wonky_set, work_report)) {
          return error(Error::already_judged);
        }

        // [GP 0.4.5 10.2 (102)/1]
        // Check if there's any misbehavior
        // (e.g. vote against good or for bad one)
        auto in_good = qtils::cxx23::ranges::contains(good_set, work_report);
        auto in_bad = qtils::cxx23::ranges::contains(bad_set, work_report);
        if ((not vote and in_bad) or (vote and in_good)) {
          return error(Error::fault_verdict_wrong);
        }

        // [GP 0.4.5 10.2 (102)/2]
        // Ensure validator from the set of current epoch
        const auto &validators_set = current_epoch_validator_set.v;
        if (not qtils::cxx23::ranges::contains_if(
                validators_set, [&](const auto &val) {
                  return val.ed25519 == validator_key;
                })) {
          return error(Error::bad_validator_index);  // TODO check error type
        }

        // [GP 0.4.5 10.2 (102)/3]
        // Ensure signature is valid
        const auto &validator_public = validator_key;
        auto is_valid_signature =
            ed25519_verify(validator_signature,
                           validator_public,
                           vote ? qtils::BytesIn{kJamValid} : kJamInvalid,
                           work_report);
        if (not is_valid_signature) {
          return error(Error::bad_signature);
        }

        // Offender signatures must be ordered by the validator’s Ed25519 key.
        // There may be no duplicate report hashes within the extrinsic, nor
        // amongst any past reported hashes.
        // [GP 0.4.5 10.2 (104)]
        if (prev_validator_key >= validator_key) {
          return error(Error::faults_not_sorted_unique);
        }
        prev_validator_key = validator_key;

        // Not report keys which are already in the punish-set
        if (qtils::cxx23::ranges::contains(punish_set, validator_key)) {
          return error(Error::offender_already_reported);
        }

        faults_registry.emplace(work_report, fault);
      }
    }

    std::unordered_map<types::WorkReportHash,
                       std::pair<types::U16, types::U16>,
                       qtils::BytesStdHash>
        vote_count_by_judgements;

    for (const auto &[work_report, judgement] : judgements_registry) {
      auto &[voted_for, voted_against] = vote_count_by_judgements[work_report];
      ++(judgement.get().vote ? voted_for : voted_against);
    }

    // Check culprits one more time, for orphan and non-bad culprits
    for (const auto &work_report : culprits_registry | std::views::keys) {
      auto it = vote_count_by_judgements.find(work_report);
      if (it == vote_count_by_judgements.end() or it->second.second == 0) {
        return error(Error::culprits_verdict_not_bad);
      }
    }

    // The state’s good-set, bad-set and wonky-set assimilate the hashes of the
    // reports from each verdict. Finally, the punish-set accumulates the keys
    // of any validators who have been found guilty of offending.

    // ψ'g - set of work-reports which were judged as correct
    // [GP 0.4.5 10.2 (112)]
    auto new_good_set = asSet(good_set);

    // ψ'b - set of work-reports which were judged as incorrect
    // [GP 0.4.5 10.2 (113)]
    auto new_bad_set = asSet(bad_set);

    // ψ'w - set of work-reports which were appeared impossible to judge
    // [GP 0.4.5 10.2 (114)]
    auto new_wonky_set = asSet(wonky_set);

    // ψ'o - a set of Ed25519 keys representing validators which were found to
    // have misjudged a work-report
    // [GP 0.4.5 10.2 (115)]
    auto new_punish_set = asSet(punish_set);

    // The offenders markers must contain exactly the keys of all new offenders,
    // respectively
    // [GP 0.4.5 10.2 (116)]
    std::vector<types::Ed25519Key> offenders_mark;

    // Analise verdicts
    for (const auto &[work_report, counts] : vote_count_by_judgements) {
      const auto &[voted_for, voted_against] = counts;

      enum Verdict { Bad, Good, Wonky } verdict;
      if (voted_against == config.validators_super_majority) {
        verdict = Bad;
      } else if (voted_for == config.validators_super_majority) {
        verdict = Good;
      } else if (voted_for == config.validators_super_majority / 2
                 and voted_against != 0) {
        verdict = Wonky;
      } else {
        return error(Error::bad_vote_split);
      }

      switch (verdict) {
        case Bad:
          // Any verdict containing solely invalid judgments implies the same
          // report having at least two valid entries in the culprits sequence
          // [GP 0.4.5 10.2 (110)]
          if (culprits_registry.count(work_report) < 2) {
            return error(Error::not_enough_culprits);
          }
          break;

        case Good:
          if (culprits_registry.count(work_report) != 0) {
            return error(Error::culprits_verdict_not_bad);
          }
          // Any verdict containing solely valid judgments implies the same
          // report having at least one valid entry in the faults sequence
          // [GP 0.4.5 10.2 (109)]
          if (faults_registry.count(work_report) < 1) {
            return error(Error::not_enough_faults);
          }
          break;

        case Wonky:
          new_wonky_set.emplace(work_report);  // [GP 0.4.5 10.2 (114)]
          break;
      }
    }

    // Analise culprits
    for (auto [work_report, culprits] : MultimapGroups{culprits_registry}) {
      // Check if the verdict of culprit is bad
      for (auto &culprit : culprits) {
        const auto &validator_public = culprit.get().key;
        offenders_mark.emplace_back(validator_public);  // [GP 0.4.5 10.2 (116)]
        new_punish_set.emplace(validator_public);       // [GP 0.4.5 10.2 (115)]
      }
      new_bad_set.emplace(work_report);  // [GP 0.4.5 10.2 (113)]
    }

    // Analise faults
    for (auto [work_report, faults] : MultimapGroups{faults_registry}) {
      // Check if the verdict of fault is bad
      for (auto &fault : faults) {
        if (fault.get().vote != false) {  // voting opposite
          return error(Error::fault_verdict_wrong);
        }
        const auto &validator_public = fault.get().key;
        offenders_mark.emplace_back(validator_public);  // [GP 0.4.5 10.2 (116)]
        new_punish_set.emplace(validator_public);       // [GP 0.4.5 10.2 (115)]
      }
      new_good_set.emplace(work_report);  // [GP 0.4.5 10.2 (112)]
    }

    // ρ - rho, aka work-reports
    auto work_reports = state.rho;

    // We clear any work-reports which we judged as uncertain or invalid from
    // their core
    // [GP 0.4.5 10.2 (111)]
    for (auto &row_work_report : work_reports.v) {
      auto work_report = mathcal_H(row_work_report->dummy_work_report);
      if (new_bad_set.contains(work_report)
          or new_wonky_set.contains(work_report)) {
        row_work_report.reset();
      }
    }

    auto state_tick = state;
    state_tick.psi.psi_g = asVec(new_good_set);
    state_tick.psi.psi_b = asVec(new_bad_set);
    state_tick.psi.psi_w = asVec(new_wonky_set);
    state_tick.psi.psi_o = asVec(new_punish_set);
    state_tick.rho = work_reports;

    return {
        state_tick,
        types::Output{types::DisputesOutputMarks{
            .offenders_mark = {offenders_mark.begin(), offenders_mark.end()},
        }},
    };
  }
}  // namespace jam::disputes

#!/usr/bin/env bash
set -euo pipefail
# set -x

init_vcpkg() {
  if [[ ! -e $VCPKG ]]; then
    git clone https://github.com/microsoft/vcpkg.git $VCPKG
  fi
  if [[ ! -e $VCPKG/vcpkg ]]; then
    $VCPKG/bootstrap-vcpkg.sh -disableMetrics
  fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  echo "This script is intended to be sourced or used as a library."
  echo "Exported functions: init_vcpkg"
  exit 1
fi

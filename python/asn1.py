#!/usr/bin/env python3

import asn1tools
import os.path
import sys

DIR = os.path.dirname(__file__)
TEST_VECTORS_DIR = os.path.join(DIR, "../test-vectors")


def flatten(aaa: list[list]):
    return [a for aa in aaa for a in aa]


def asn_file(name: str):
    return os.path.join(TEST_VECTORS_DIR, "jamtestvectors", name + ".asn")


def write(path: str, lines: list[str]):
    with open(path, "w") as f:
        f.writelines(line + "\n" for line in lines)


def c_using(name: str, other: str):
    return ["using %s = %s;" % (name, other)]


def c_struct(name: str, members):
    return [
        "struct %s {" % name,
        *(("  %s %s;" % (t, k) for k, t in members)),
        "  bool operator==(const %s &) const = default;" % name,
        "};",
    ]


def c_dash(s: int | str):
    if isinstance(s, int):
        return s
    return s.replace("-", "_")


def indent(lines):
    return ["  " + x for x in lines]


class Type:
    ignore_args = True

    def __init__(self, name: str):
        self.name = name
        self.args: list[str] = []
        self.decl: list[str] = []
        # self.scale: list[str] = []
        self.diff: list[str] = []

    def c_tdecl(self):
        if self.ignore_args or not self.args:
            return []
        return [
            "template<%s>" % ", ".join("uint32_t %s" % c_dash(x) for x in self.args)
        ]

    def c_targs(self):
        if self.ignore_args or not self.args:
            return ""
        return "<%s>" % ", ".join(c_dash(x) for x in self.args)

    def c_tname(self):
        return self.name + self.c_targs()


def asn_recurse(t: dict):
    yield t
    e = t.get("element", None)
    if e:
        yield from asn_recurse(e)
    m = t.get("members", None)
    if m:
        for x in m:
            yield from asn_recurse(x)


def asn_deps(types: dict):
    def deep(k: str):
        yield k
        for dep in deps1[k]:
            yield from deep(dep)

    deps1 = {
        k: {x for x in (x["type"] for x in asn_recurse(t)) if x in types}
        for k, t in types.items()
    }
    deps1 = {k: set(flatten(map(deep, deps))) for k, deps in deps1.items()}
    deps2: dict[str, set[Type]] = {k: set() for k in types}
    for k, deps in deps1.items():
        for dep in deps:
            deps2[dep].add(k)
    return deps1, deps2


def asn_args(ARGS: list[str], types: dict, deps2: dict[str, set[str]]):
    args = {
        k: {
            s[0]
            for s in (x.get("size", None) for x in asn_recurse(t))
            if s and isinstance(s[0], str)
        }
        for k, t in types.items()
    }
    for tname, deps in deps2.items():
        for dep in deps:
            args[dep].update(args[tname])
    return {k: [a for a in ARGS if a in aa] for k, aa in args.items()}


# def c_scale(ty: Type, encode: list[str], decode: list[str]):
#     return [
#         *ty.c_tdecl(),
#         "inline scale::ScaleEncoderStream &operator<<(scale::ScaleEncoderStream &s, const %s &v) {"
#         % ty.c_tname(),
#         *indent(encode),
#         "  return s;",
#         "}",
#         *ty.c_tdecl(),
#         "inline scale::ScaleDecoderStream &operator>>(scale::ScaleDecoderStream &s, %s &v) {"
#         % ty.c_tname(),
#         *indent(decode),
#         "  return s;",
#         "}",
#     ]
#
#
# def c_scale_struct(ty: Type, members: list[str]):
#     return c_scale(
#         ty,
#         ["s << v.%s;" % x for x in members],
#         ["s >> v.%s;" % x for x in members],
#     )


def c_diff(NS: str, ty: Type, lines: list[str]):
    return [
        *ty.c_tdecl(),
        "DIFF_F(%s::%s) {" % (NS, ty.c_tname()),
        *indent(lines),
        "}",
    ]


def c_fittest_int_type(min_value: int, max_value: int):
    if min_value <= max_value:
        if min_value >= 0:
            if max_value <= 255: return "uint8_t"
            if max_value <= 65536: return "uint16_t"
            if max_value <= 4294967295: return "uint32_t"
            if max_value <= 18446744073709551615: return "uint64_t"
        else:  # min_value < 0:
            if min_value >= -128 and max_value <= 127: return "int8_t"
            if min_value >= -32768 and max_value <= 32767: return "int16_t"
            if min_value >= -2147483648 and max_value <= 2147483647: return "int32_t"
            if min_value >= -9223372036854775808 and max_value <= 9223372036854775807: return "int64_t"
    return None


def parse_types(cpp_namespace: str, ARGS: list[str], path: str, key: str, imports: list[str] = []):
    def asn_sequence_of(t):
        (size,) = t.get("size", (None,))
        fixed = isinstance(size, (int, str))
        T = t["element"]["type"]
        assert T in types
        if T == "U8":
            if fixed:
                if type(size) is int:
                    return "qtils::BytesN<%u>" % size
                else:
                    return "::jam::ConfigVec<uint8_t, Config::Field::%s>" % c_dash(size)
            return "qtils::Bytes"
        if fixed:
            if isinstance(size, str):
                return "::jam::ConfigVec<%s, Config::Field::%s>" % (T, c_dash(size))
            return "std::array<%s, %s>" % (T, c_dash(size))
        return "std::vector<%s>" % T

    def asn_member(t):
        if t["type"] == "INTEGER":
            int_type = c_fittest_int_type(*t["restricted-to"][0])
            if int_type is None: raise TypeError(t)
            return int_type
        if t["type"] == "BOOLEAN":
            return "bool"
        if t["type"] == "OCTET STRING":
            if 'size' in t:
                t = dict(type="SEQUENCE OF", element=dict(type="U8"), size=t["size"])
            else:
                t = dict(type="SEQUENCE OF", element=dict(type="U8"))
        if t["type"] == "NULL":
            if "tag" in t:
                if "number" in t["tag"]:
                    r = "Tagged<Empty, struct _%d>" % t["tag"]["number"]
                else:
                    r = "Tagged<Empty, struct %s>" % c_dash(t["tag"]["str"])
            else:
                r = "Empty"
        elif t["type"] == "SEQUENCE OF":
            r = asn_sequence_of(t)
        elif t["type"] in types:
            r = types[t["type"]].c_tname()
        else:
            raise TypeError(t)
        if t.get("optional", False):
            return "std::optional<%s>" % r
        return r

    asn_types: dict = asn1tools.parse_files([path])[key]["types"]
    deps1, deps2 = asn_deps(asn_types)

    types = {tname: Type(tname) for tname in asn_types}
    types.update({tname: Type(tname) for tname in imports})
    for tname, args in asn_args(ARGS, asn_types, deps2).items():
        types[tname].args = args

    enum_trait = []
    for tname, t in asn_types.items():
        ty = types[tname]
        if t["type"] == "CHOICE":
            if [x["name"] for x in t["members"]] == ["none", "some"]:
                if [x["tag"]["number"] for x in t["members"]] == [0, 1]:
                    x = t["members"][1]
                    ty.decl = c_using(
                        tname,
                        "std::optional<%s>" % asn_member(t["members"][1])
                    )
                    continue
            ty.decl = [
                "using %s = Tagged<std::variant<" % tname,
                *["  %s, // %s" % (asn_member(x), c_dash(x["name"])) for x in t["members"][:-1]],
                *["  %s // %s" % (asn_member(x), c_dash(x["name"])) for x in t["members"][-1:]],
                ">, struct %s_Tag>;" % tname
            ]
            ty.diff = [
                "DIFF_F(%s::%s) {" % (cpp_namespace, tname),
                "  static constexpr std::string_view tags[] = {",
                *['    "%s",' % x["name"] for x in t["members"][:-1]],
                *['    "%s"' % x["name"] for x in t["members"][-1:]],
                "  };",
                "  diff_v(indent, v1, v2, std::span(tags));",
                "}"
            ]
            continue
        if t["type"] == "ENUMERATED":
            base_type = c_fittest_int_type(min((x for _, x in t["values"]), default=0),
                                           max((x for _, x in t["values"]), default=0))
            if base_type is None: raise TypeError(t)
            ty.decl = [
                "enum class %s : %s {" % (tname, base_type),
                *("  %s = %d," % (c_dash(name), index) for name, index in t["values"][:-1]),
                *("  %s = %d" % (c_dash(name), index) for name, index in t["values"][-1:]),
                "};",
            ]
            ty.diff = [
                "DIFF_F(%s::%s) {" % (cpp_namespace, tname),
                "  static const std::unordered_map<%s, std::string_view> names = {" % base_type,
                *('    {%d, "%s"},' % (num, name) for name, num in t["values"][:-1]),
                *('    {%d, "%s"}' % (num, name) for name, num in t["values"][-1:]),
                "  };",
                "  diff_e(indent, v1, v2, names);",
                "}",
            ]
            enum_trait.append(
                "SCALE_DEFINE_ENUM_VALUE_LIST(%s, %s, %s)"
                % (
                    cpp_namespace,
                    ty.name,
                    ", ".join(
                        "%s::%s::%s" % (cpp_namespace, ty.name, c_dash(x[0])) for x in t["values"]
                    ),
                )
            )
            continue
        if t["type"] == "SEQUENCE":
            ty.decl = c_struct(
                tname, [(c_dash(x["name"]), asn_member(x)) for x in t["members"]]
            )
            # ty.scale += c_scale_struct(ty, [c_dash(x["name"]) for x in t["members"]])
            # ty.diff = c_diff(
            #     cpp_namespace, ty, ["DIFF_M(%s);" % c_dash(x["name"]) for x in t["members"]]
            # )
            ty.diff = c_diff(
                cpp_namespace, ty, ["DIFF_M(%s);" % c_dash(x["name"]) for x in t["members"]]
            )
            continue
        if t["type"] == "NULL":
            ty.decl = c_using(tname, "Empty");
            continue
        ty.decl = c_using(tname, asn_member(t))

    order = asn1tools.c.utils.topological_sort(deps1)
    return [types[k] for k in order], enum_trait


def parse_const(path: str, key: str):
    values: dict = asn1tools.parse_files([path])[key]["values"]
    assert all(v["type"] == "INTEGER" for v in values.values())
    return {k: v["value"] for k, v in values.items()}


class GenConstants:
    def __init__(self, cpp_namespace: str, path: str):
        config_asn = [
            (name, parse_const(asn_file("%s/%s-const" % (path, name)), "Constants"))
            for name in ["tiny", "full"]
        ]

        constants = dict((c_dash(a), dict([(c_dash(k), v) for (k, v) in b.items()])) for (a, b) in config_asn)

        set_names = [x for x in constants.keys()]
        constant_names = [c_dash(x) for x in next(iter(constants.values())).keys()]

        self.struct = [
            "// Auto-generated file",
            "",
            "#pragma once",
            "",
            "#include <cstdint>",
            "",
            "namespace %s {" % cpp_namespace,
            ""
            "  struct Config {",
            "",
            "    struct Field {",
            *["      struct %s {};" % const_name for const_name in constant_names],
            "    };",
            *["    uint32_t %s;" % const_name for const_name in constant_names],
            *[
                "    auto get(Field::%s) const { return %s; }"
                % (const_name, const_name)
                for const_name in constant_names
            ],
            "  };",
            "",
            "}  // namespace %s" % cpp_namespace
        ]

        self.constants = dict()
        self.configs = dict()
        for (set_name, pairs) in constants.items():
            content = [
                "// Auto-generated file",
                "",
                "#pragma once",
                "",
                "#include <cstdint>",
                "",
                "namespace %s::constants::%s {" % (cpp_namespace, set_name),
                ""
            ]
            content += ["  constexpr uint32_t %s = %d;" % (name, value) for (name, value) in pairs.items()]
            content += [
                "",
                "}  // namespace %s" % cpp_namespace
            ]
            self.constants[set_name] = content

            content = [
                "// Auto-generated file",
                "",
                "#pragma once",
                "",
                "#include <test-vectors/config.hpp>",
                "#include <test-vectors/constants-%s.hpp>" % set_name,
                "",
                "namespace %s::config {" % cpp_namespace,
                "",
                "  constexpr Config %s {" % set_name,
                *["    .%s = constants::%s::%s," % (name, set_name, name) for name in constant_names],
                "  };",
                "",
                "};",
            ]
            self.configs[set_name] = content

    def write(self):
        prefix = os.path.join(TEST_VECTORS_DIR)
        write(prefix + "/config.hpp", self.struct)
        for (set_name, content) in self.constants.items():
            write(prefix + "/constants-%s.hpp" % set_name, content)
        for (set_name, content) in self.configs.items():
            write(prefix + "/config-%s.hpp" % set_name, content)


class GenCommonTypes:
    def __init__(self, cpp_namespace: str, path: str):
        self.types, self.enum_trait = parse_types(cpp_namespace, [], asn_file("%s/jam-types" % path), "JamTypes")
        self.g_types = flatten([*ty.c_tdecl(), *ty.decl] for ty in self.types)
        self.g_types = ["namespace %s {" % cpp_namespace, *indent(self.g_types), "}"]
        self.g_types = [
            "// Auto-generated file",
            "",
            "#pragma once",
            "",
            "#include <array>",
            "#include <optional>",
            "#include <string_view>",
            "#include <variant>",
            "",
            "#include <qtils/bytes.hpp>",
            "",
            "#include <test-vectors/config-types.hpp>",
            "#include <test-vectors/config.hpp>",
            "#include <src_/jam/empty.hpp>",
            "#include <src_/jam/tagged.hpp>",
            "",
            *self.g_types,
        ]
        # self.g_scale = flatten([ty.scale for ty in self.types])
        # self.g_scale = [
        #     "namespace %s {" % cpp_namespace,
        #     *indent(self.g_scale), "}"
        # ]
        # self.g_scale = [
        #     "// Auto-generated file",
        #     "",
        #     "#pragma once",
        #     "",
        #     "#include <scale/scale.hpp>",
        #     "#include <src_/TODO_scale/aggregate.hpp>",
        #     "",
        #     "#include <test-vectors/config-types-scale.hpp>",
        #     '#include <test-vectors/common-types.hpp>',
        #     "",
        #     *self.g_scale,
        #     *self.enum_trait,
        # ]
        self.g_diff = flatten([ty.diff for ty in self.types])
        self.g_diff = [
            "// Auto-generated file",
            "",
            "#pragma once",
            "",
            "#include <test-vectors/diff.hpp>",
            '#include <test-vectors/common-types.hpp>',
            "",
            *self.g_diff,
        ]

    def write(self):
        prefix = os.path.join(TEST_VECTORS_DIR)
        write(prefix + "/common-types.hpp", self.g_types)
        # write(prefix + "/common-scale.hpp", self.g_scale)
        write(prefix + "/common-diff.hpp", self.g_diff)


class GenSpecialTypes:
    def __init__(self, cpp_namespace: str, name: str, path: str, module: str):
        self.asn_file = asn_file(path)

        asn_imports: dict = asn1tools.parse_files([self.asn_file])[module]["imports"]

        includes = []
        usings = []
        for module_name, importing_types in asn_imports.items():
            match module_name:
                case 'JamTypes':
                    includes += ["#include <test-vectors/common-types.hpp>"]
                    usings += ["using ::%s::%s;" % (cpp_namespace, t) for t in importing_types]

        self.types, self.enum_trait = parse_types("%s::%s" % (cpp_namespace, name), [], self.asn_file, module,
                                                  flatten(asn_imports.values()))
        self.g_types = flatten([[*ty.c_tdecl(), *ty.decl] for ty in self.types])
        self.g_types = ["namespace %s::%s {" % (cpp_namespace, name), "", *indent(usings), "", *indent(self.g_types),
                        "", "}"]
        self.g_types = [
            "// Auto-generated file",
            "",
            "#pragma once",
            "",
            "#include <array>",
            "#include <optional>",
            "#include <string_view>",
            "#include <variant>",
            "",
            "#include <qtils/bytes.hpp>",
            "",
            "#include <test-vectors/config-types.hpp>",
            "#include <test-vectors/config.hpp>",
            *includes,
            "",
            *self.g_types,
        ]
        # self.g_scale = flatten([ty.scale for ty in self.types])
        # self.g_scale = ["namespace %s::%s {" % (cpp_namespace, name), "", *indent(self.g_scale), "", "}"]
        # self.g_scale = [
        #     "// Auto-generated file",
        #     "",
        #     "#pragma once",
        #     "",
        #     "#include <scale/scale.hpp>",
        #     "",
        #     '#include <test-vectors/common-scale.hpp>',
        #     '#include <test-vectors/%s/%s-types.hpp>' % (name, name),
        #     "",
        #     *self.g_scale,
        #     *self.enum_trait,
        # ]
        self.g_diff = flatten([ty.diff for ty in self.types])
        self.g_diff = [
            "// Auto-generated file",
            "",
            "#pragma once",
            "",
            "#include <test-vectors/diff.hpp>",
            '#include <test-vectors/common-diff.hpp>',
            '#include <test-vectors/%s/%s-types.hpp>' % (name, name),
            "",
            *self.g_diff,
        ]

    def write(self, name: str):
        prefix = os.path.join(TEST_VECTORS_DIR, name, name)
        write(prefix + "-types.hpp", self.g_types)
        # write(prefix + "-scale.hpp", self.g_scale)
        write(prefix + "-diff.hpp", self.g_diff)


def constants():
    g = GenConstants(
        "jam::test_vectors",
        "jam-types-asn",
    )
    g.write()


def types():
    g = GenCommonTypes(
        "jam::test_vectors",
        "jam-types-asn",
    )
    g.write()


def history():
    g = GenSpecialTypes(
        "jam::test_vectors",
        "history",
        "history/history",
        "HistoryModule",
    )
    g.write("history")


def safrole():
    g = GenSpecialTypes(
        "jam::test_vectors",
        "safrole",
        "safrole/safrole",
        "SafroleModule",
    )
    g.write("safrole")


def disputes():
    g = GenSpecialTypes(
        "jam::test_vectors",
        "disputes",
        "disputes/disputes",
        "DisputesModule",
    )
    g.write("disputes")


def authorizations():
    g = GenSpecialTypes(
        "jam::test_vectors",
        "authorizations",
        "authorizations/authorizations",
        "AuthorizationsModule",
    )
    g.write("authorizations")


if __name__ == "__main__":
    for arg in sys.argv[1:]:
        dict(
            constants=constants,
            types=types,
            history=history,
            safrole=safrole,
            disputes=disputes,
            authorizations=authorizations,
        )[arg]()

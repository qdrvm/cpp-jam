#!/usr/bin/env python3

import asn1tools
import os.path

DIR = os.path.dirname(__file__)
TEST_VECTORS_DIR = os.path.join(DIR, "../test-vectors")
OUTPUT_NAME = "types"
OUTPUT_DIR = os.path.join(TEST_VECTORS_DIR, "safrole")
SAFROLE_DIR = os.path.join(TEST_VECTORS_DIR, "jamtestvectors/safrole")
ARGS = ["validators-count", "epoch-length"]
NS = "jam::test_vectors_safrole"


def flatten(aaa: list[list]):
    return [a for aa in aaa for a in aa]


def safrole_file(name: str):
    return os.path.join(SAFROLE_DIR, name + ".asn")


def write(ext: str, lines: list[str]):
    with open(os.path.join(OUTPUT_DIR, OUTPUT_NAME + ext), "w") as f:
        f.writelines(line + "\n" for line in lines)


def c_using(name: str, other: str):
    return ["using %s = %s;" % (name, other)]


def c_struct(name: str, members):
    return ["struct %s {" % name, *(("  %s %s;" % (t, k) for k, t in members)), "};"]


def c_dash(s: int | str):
    if isinstance(s, int):
        return s
    return s.replace("-", "_")


def indent(lines):
    return ["  " + x for x in lines]


class Type:
    def __init__(self, name: str):
        self.name = name
        self.args: list[str] = []
        self.decl: list[str] = []
        self.scale: list[str] = []

    def c_tdecl(self):
        if not self.args:
            return []
        return ["template<%s>" % ", ".join("size_t %s" % c_dash(x) for x in self.args)]

    def c_targs(self):
        if not self.args:
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


def asn_args(types: dict, deps2: dict[str, set[str]]):
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


def c_scale(ty: Type, encode: list[str], decode: list[str]):
    return [
        *ty.c_tdecl(),
        "inline scale::ScaleEncoderStream &operator<<(scale::ScaleEncoderStream &s, const %s &v) {"
        % ty.c_tname(),
        *indent(encode),
        "  return s;",
        "}",
        *ty.c_tdecl(),
        "inline scale::ScaleDecoderStream &operator>>(scale::ScaleDecoderStream &s, %s &v) {"
        % ty.c_tname(),
        *indent(decode),
        "  return s;",
        "}",
    ]


def c_scale_struct(ty: Type, members: list[str]):
    return c_scale(
        ty,
        ["s << v.%s;" % x for x in members],
        ["s >> v.%s;" % x for x in members],
    )


def parse_types(path: str):
    def asn_sequence_of(t):
        (size,) = t["size"]
        fixed = isinstance(size, (int, str))
        T = t["element"]["type"]
        assert T in asn_types
        if T == "U8":
            if fixed:
                return "qtils::BytesN<%s>" % c_dash(size)
            return "qtils::Bytes"
        if fixed:
            return "std::array<%s, %s>" % (T, c_dash(size))
        return "std::vector<%s>" % T

    def asn_member(t):
        if t["type"] == "SEQUENCE OF":
            r = asn_sequence_of(t)
        elif t["type"] in asn_types:
            r = types[t["type"]].c_tname()
        else:
            raise TypeError(t)
        if t.get("optional", False):
            return "std::optional<%s>" % r
        return r

    asn_types: dict = asn1tools.parse_files([path])["SafroleModule"]["types"]
    deps1, deps2 = asn_deps(asn_types)
    types = {tname: Type(tname) for tname in asn_types}
    for tname, args in asn_args(asn_types, deps2).items():
        types[tname].args = args
    enum_trait = []
    for tname, t in asn_types.items():
        ty = types[tname]
        if tname == "U8":
            ty.decl = c_using(tname, "uint8_t")
            continue
        if tname == "U32":
            ty.decl = c_using(tname, "uint32_t")
            continue
        if t["type"] == "CHOICE":
            ty.decl = c_struct(
                tname,
                [
                    (
                        "v",
                        "boost::variant<%s>"
                        % ", ".join(asn_member(x) for x in t["members"]),
                    )
                ],
            )
            ty.scale.extend(c_scale_struct(ty, ["v"]))
            continue
        if t["type"] == "ENUMERATED":
            values = [x[1] for x in t["values"]]
            assert all(x == i and x < 0xFF for i, x in enumerate(values))
            ty.decl = [
                "enum class %s : uint8_t {" % tname,
                *("  %s," % c_dash(x[0]) for x in t["values"]),
                "};",
            ]
            ns = "%s::generic" % NS
            enum_trait.append(
                "SCALE_DEFINE_ENUM_VALUE_LIST(%s, %s, %s)"
                % (
                    ns,
                    ty.name,
                    ", ".join(
                        "%s::%s::%s" % (ns, ty.name, c_dash(x[0])) for x in t["values"]
                    ),
                )
            )
            continue
        if t["type"] == "SEQUENCE":
            ty.decl = c_struct(
                tname, [(c_dash(x["name"]), asn_member(x)) for x in t["members"]]
            )
            ty.scale.extend(
                c_scale_struct(ty, [c_dash(x["name"]) for x in t["members"]])
            )
            continue
        ty.decl = c_using(tname, asn_member(t))

    order = asn1tools.c.utils.topological_sort(deps1)
    return [types[k] for k in order], enum_trait


def parse_const(path: str):
    values: dict = asn1tools.parse_files([path])["SafroleConstants"]["values"]
    assert all(v["type"] == "INTEGER" for v in values.values())
    return {k: v["value"] for k, v in values.items()}


types, enum_trait = parse_types(safrole_file("safrole"))


g_types = flatten([*ty.c_tdecl(), *ty.decl] for ty in types)
g_types = ["namespace %s::generic {" % NS, *indent(g_types), "}"]
for name in ["tiny", "full"]:
    args = parse_const(safrole_file(name))
    g_args = [
        *["static constexpr size_t %s = %s;" % (c_dash(a), args[a]) for a in ARGS],
        *flatten(
            c_using(ty.name, "%s::generic::%s" % (NS, ty.c_tname())) for ty in types
        ),
    ]
    g_args = ["struct %s {" % name, *indent(g_args), "};"]
    g_types.extend(
        [
            "namespace %s {" % NS,
            *indent(g_args),
            "}",
        ]
    )
g_types = [
    "#pragma once",
    "#include <boost/variant.hpp>",
    "#include <qtils/bytes.hpp>",
    *g_types,
]

g_scale = flatten(ty.scale for ty in types)
g_scale = ["namespace %s::generic {" % NS, *indent(g_scale), "}"]
g_scale = [
    "#pragma once",
    "#include <scale/scale.hpp>",
    '#include "%s.hpp"' % OUTPUT_NAME,
    *g_scale,
    *enum_trait,
]

write(".hpp", g_types)
write(".scale.hpp", g_scale)

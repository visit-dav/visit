#!/usr/bin/env python3
from __future__ import annotations

import argparse
import pathlib
import re
from dataclasses import dataclass


def _apply_case(replacement: str, original: str) -> str:
    if original.isupper():
        return replacement.upper()
    if original[:1].isupper():
        return replacement[:1].upper() + replacement[1:]
    return replacement


@dataclass(frozen=True)
class Rule:
    pattern: re.Pattern[str]
    repl: object  # str | Callable[[re.Match[str]], str]
    description: str


_ADJACENT_NOUNS = (
    "option",
    "options",
    "argument",
    "arguments",
    "flag",
    "flags",
    "parameter",
    "parameters",
    "interface",
    "tool",
    "tools",
    "utility",
    "utilities",
    "program",
    "programs",
    "syntax",
    "usage",
    "switch",
    "switches",
)


def _fix_command_line(text: str) -> str:
    adjacent = "|".join(_ADJACENT_NOUNS)

    # Adjective: "command line option" -> "command-line option"
    adjective = re.compile(rf"\bcommand[ -]line(?=\s+(?:{adjacent})\b)", re.IGNORECASE)
    text = adjective.sub(lambda m: _apply_case("command-line", m.group(0)), text)

    # Noun: any remaining "command-line" becomes "command line"
    noun = re.compile(rf"\bcommand-line\b(?!\s+(?:{adjacent})\b)", re.IGNORECASE)
    return noun.sub("command line", text)


_RULES: list[Rule] = [
    Rule(
        re.compile(r"\bplug-in(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"plugin{m.group(1) or ''}", m.group(0)),
        "plugin",
    ),
    Rule(
        re.compile(r"\bsub[- ]tree\b", re.IGNORECASE),
        lambda m: _apply_case("subtree", m.group(0)),
        "subtree",
    ),
    Rule(
        re.compile(r"\bmeta[- ]data\b", re.IGNORECASE),
        lambda m: _apply_case("metadata", m.group(0)),
        "metadata",
    ),
    Rule(
        re.compile(r"\btime[- ]step(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"timestep{m.group(1) or ''}", m.group(0)),
        "timestep",
    ),
    Rule(
        re.compile(r"\btime[- ]stamp(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"timestamp{m.group(1) or ''}", m.group(0)),
        "timestamp",
    ),
    Rule(
        re.compile(r"\bdata[- ]type(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"datatype{m.group(1) or ''}", m.group(0)),
        "datatype",
    ),
    Rule(
        re.compile(r"\bdata[- ]set(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"dataset{m.group(1) or ''}", m.group(0)),
        "dataset",
    ),
    Rule(
        re.compile(r"\bfile[- ]system(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"filesystem{m.group(1) or ''}", m.group(0)),
        "filesystem",
    ),
    Rule(
        re.compile(r"\bwork[- ]load(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"workload{m.group(1) or ''}", m.group(0)),
        "workload",
    ),
    Rule(
        re.compile(r"\bwork[- ]flow(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"workflow{m.group(1) or ''}", m.group(0)),
        "workflow",
    ),
    Rule(
        re.compile(r"\bmulti[- ]dimensional\b", re.IGNORECASE),
        lambda m: _apply_case("multidimensional", m.group(0)),
        "multidimensional",
    ),
    Rule(
        re.compile(r"\bnon[- ]linear\b", re.IGNORECASE),
        lambda m: _apply_case("nonlinear", m.group(0)),
        "nonlinear",
    ),
    Rule(
        re.compile(r"\bpre[- ]compute(d|s|ing)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"precompute{m.group(1) or ''}", m.group(0)),
        "precompute",
    ),
    Rule(
        re.compile(r"\bpre[- ]compile(d|s|ing)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"precompile{m.group(1) or ''}", m.group(0)),
        "precompile",
    ),
    Rule(
        re.compile(r"\bre-(open|start|build|write|do)([a-z]+)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"re{m.group(1)}{m.group(2) or ''}", m.group(0)),
        "reopen/restart/rebuild/rewrite/redo",
    ),
    Rule(
        re.compile(r"\brun[ -]?time(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"runtime{m.group(1) or ''}", m.group(0)),
        "runtime",
    ),
    Rule(
        re.compile(r"\bmulti[ -]?resolution(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"multiresolution{m.group(1) or ''}", m.group(0)),
        "multiresolution",
    ),
    Rule(
        re.compile(r"\bmulti[ -]?block(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"multiblock{m.group(1) or ''}", m.group(0)),
        "multiblock",
    ),
    Rule(
        re.compile(r"\btime[ -]?state(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"time-state{m.group(1) or ''}", m.group(0)),
        "time-state",
    ),
    Rule(
        re.compile(r"\bhigh[ -]?level\b", re.IGNORECASE),
        lambda m: _apply_case("high-level", m.group(0)),
        "high-level",
    ),
    Rule(
        re.compile(r"\blow[ -]?level\b", re.IGNORECASE),
        lambda m: _apply_case("low-level", m.group(0)),
        "low-level",
    ),
    Rule(
        re.compile(r"\bthird[ -]?party\b", re.IGNORECASE),
        lambda m: _apply_case("third-party", m.group(0)),
        "third-party",
    ),
    Rule(
        re.compile(r"\bproblem[ -]?sized\b", re.IGNORECASE),
        lambda m: _apply_case("problem-sized", m.group(0)),
        "problem-sized",
    ),
    Rule(
        re.compile(r"\b(zone|node|edge|face|vertex|cell)[ -]?centered\b", re.IGNORECASE),
        lambda m: f"{m.group(1)}-centered",
        "zone/node/edge/face/vertex/cell-centered",
    ),
    Rule(
        re.compile(r"\bblock[ -]?structured\b", re.IGNORECASE),
        lambda m: _apply_case("block-structured", m.group(0)),
        "block-structured",
    ),
    Rule(
        re.compile(r"\bpatch[ -]?based\b", re.IGNORECASE),
        lambda m: _apply_case("patch-based", m.group(0)),
        "patch-based",
    ),
    Rule(
        re.compile(r"\bmulti[ -]?domain(s)?\b", re.IGNORECASE),
        lambda m: _apply_case(f"multi-domain{m.group(1) or ''}", m.group(0)),
        "multi-domain",
    ),
    Rule(
        re.compile(r"\bstaggered[ -]?grid\b", re.IGNORECASE),
        lambda m: _apply_case("staggered grid", m.group(0)),
        "staggered grid",
    ),
]


_INLINE_PROTECT_PATTERNS: list[re.Pattern[str]] = [
    re.compile(r"``[^`]*``"),  # inline literals
    re.compile(r":[A-Za-z0-9_+:-]+:`[^`]*`"),  # roles like :file:`...`
    re.compile(r"`[^`]*`"),  # interpreted text (often code-ish)
    re.compile(r"\b(?:https?://|ftp://|file://|mailto:|www\.)\S+\b"),
    re.compile(r"\S*[/\\]\S*"),  # filesystem-ish paths or urls
]


def _merge_ranges(ranges: list[tuple[int, int]]) -> list[tuple[int, int]]:
    if not ranges:
        return []
    ranges.sort()
    merged: list[tuple[int, int]] = [ranges[0]]
    for start, end in ranges[1:]:
        last_start, last_end = merged[-1]
        if start <= last_end:
            merged[-1] = (last_start, max(last_end, end))
        else:
            merged.append((start, end))
    return merged


def _protected_ranges(line: str) -> list[tuple[int, int]]:
    ranges: list[tuple[int, int]] = []
    for pattern in _INLINE_PROTECT_PATTERNS:
        for match in pattern.finditer(line):
            ranges.append((match.start(), match.end()))
    return _merge_ranges(ranges)


def _fix_all(text: str) -> str:
    text = _fix_command_line(text)
    for rule in _RULES:
        text = rule.pattern.sub(rule.repl, text)  # type: ignore[arg-type]
    return text


def _apply_rules_to_unprotected_text(line: str) -> str:
    protected = _protected_ranges(line)
    if not protected:
        return _fix_all(line)

    out_parts: list[str] = []
    cursor = 0
    for start, end in protected:
        if cursor < start:
            out_parts.append(_fix_all(line[cursor:start]))
        out_parts.append(line[start:end])
        cursor = end
    if cursor < len(line):
        out_parts.append(_fix_all(line[cursor:]))
    return "".join(out_parts)


_DIRECTIVE_THAT_PROTECTS = re.compile(
    r"^\s*\.\.\s+(code-block|code|literalinclude|parsed-literal|math|graphviz|dot|mermaid)::"
)
_ANY_DIRECTIVE = re.compile(r"^\s*\.\.\s+\S+::")


def _fix_rst_contents(contents: str) -> str:
    lines = contents.splitlines(keepends=True)

    in_protected_block = False
    protected_base_indent = 0
    pending_literal_from_colons = False
    pending_literal_base_indent = 0

    out_lines: list[str] = []
    for line in lines:
        stripped = line.rstrip("\n")
        indent = len(stripped) - len(stripped.lstrip(" "))
        is_blank = stripped.strip() == ""

        if in_protected_block:
            if is_blank or indent > protected_base_indent:
                out_lines.append(line)
                continue
            in_protected_block = False

        if pending_literal_from_colons:
            if is_blank:
                out_lines.append(line)
                continue
            if indent > pending_literal_base_indent:
                in_protected_block = True
                protected_base_indent = pending_literal_base_indent
                pending_literal_from_colons = False
                out_lines.append(line)
                continue
            pending_literal_from_colons = False

        if _DIRECTIVE_THAT_PROTECTS.match(stripped):
            in_protected_block = True
            protected_base_indent = indent
            out_lines.append(line)
            continue

        if stripped.rstrip().endswith("::") and not _ANY_DIRECTIVE.match(stripped):
            pending_literal_from_colons = True
            pending_literal_base_indent = indent
            out_lines.append(line)
            continue

        if stripped.lstrip().startswith(".. _") and "://" in stripped:
            # Link target lines like ".. _name: https://example.com"
            out_lines.append(line)
            continue

        out_lines.append(_apply_rules_to_unprotected_text(line))

    return "".join(out_lines)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Normalize hyphenation for selected terms in RST files (skips code blocks, URLs, and paths)."
    )
    parser.add_argument(
        "root",
        nargs="?",
        default="src/doc",
        help="Root directory to scan (default: src/doc)",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Do not write files; exit non-zero if changes would be made.",
    )
    args = parser.parse_args()

    root = pathlib.Path(args.root)
    rst_files = sorted(root.rglob("*.rst"))

    changed_files: list[pathlib.Path] = []
    for path in rst_files:
        original = path.read_text(encoding="utf-8", errors="surrogateescape")
        fixed = _fix_rst_contents(original)
        if fixed != original:
            changed_files.append(path)
            if not args.check:
                path.write_text(fixed, encoding="utf-8", errors="surrogateescape")

    if args.check:
        if changed_files:
            for path in changed_files:
                print(path)
            return 1
        return 0

    for path in changed_files:
        print(path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())


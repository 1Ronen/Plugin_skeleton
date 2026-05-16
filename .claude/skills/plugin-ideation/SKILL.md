---
name: plugin-ideation
description: >
  Capture plugin concept into .ideas/ contracts. Generates creative-brief.md,
  parameter-spec.md, architecture.md, and plan.md. Use when the user says
  "I want to make a plugin that...", "brainstorm", "what parameters should X have",
  or any creative/planning phase before implementation.
allowed-tools:
  - Read
  - Write
  - Edit
---

# plugin-ideation

Structured brainstorming that produces concrete `.ideas/` contracts, ready for
`/new-plugin` and the foundation-agent.

## Workflow

### Phase 1 — Concept capture

Ask the user (or extract from their description) the core idea in one sentence:

> "A [type] plugin that [primary sonic goal], controlled by [key parameters]."

Identify:
- Plugin type (effect, dynamics, EQ, distortion, reverb, delay, utility, instrument)
- Primary sonic goal
- 2–4 key parameters (rough, refined later)

### Phase 2 — Gap analysis

For each gap below, generate 1–2 targeted questions:

**Tier 1 — Blocking (must resolve):**
- [ ] Plugin name (CamelCase identifier)
- [ ] Primary parameter list (IDs, types, ranges)
- [ ] Signal chain (stages, order)

**Tier 2 — Important:**
- [ ] UI control types (knobs vs sliders vs toggles)
- [ ] Inspirations / reference plugins
- [ ] Out-of-scope for v1

**Tier 3 — Nice to have:**
- [ ] Complexity score (1–10, user's gut feel)
- [ ] Edge cases / unusual parameter ranges

Ask no more than 4 questions per round. After answers, re-assess gaps and iterate
until all Tier 1 gaps are resolved.

### Phase 3 — Generate contracts

When Tier 1 gaps are resolved, generate all four contract files.

**creative-brief.md** — Vision, use cases, inspirations.

**parameter-spec.md** — One section per parameter. For each:
- ID (snake_case, no spaces)
- Type (Float/Bool/Choice)
- Range and default
- Unit suffix
- DSP role

**architecture.md** — Signal chain diagram, DSP component list, GUI layout.

**plan.md** — Phase breakdown, complexity score, risk items.

Write to `.ideas/[contract].md` using the templates in `.ideas/`.

If a plugin-specific `.ideas/` exists (e.g. `plugins/[PluginName]/.ideas/`),
write there instead.

### Phase 4 — Review

Present a summary of what was generated. Ask:

> "Anything to adjust before we proceed to implementation?"

If yes, iterate on specific contracts. If no, confirm contracts are ready.

### Phase 5 — Handoff

```
Contracts ready in .ideas/:
  creative-brief.md   ✓
  parameter-spec.md   ✓
  architecture.md     ✓
  plan.md             ✓

Next: /new-plugin to instantiate the template
```

## Style Rules

- Parameter IDs: `snake_case`, descriptive, max 12 chars
- No redundant parameters (if one controls another, collapse them)
- Float defaults should be musically useful out of the box (not min/max)
- Complexity score 1–3: weekend project; 4–6: one week; 7–10: multi-week

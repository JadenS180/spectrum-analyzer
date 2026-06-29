# 🏀 Ethical Hoops Index (EHI)

A per-game NBA player ethics metric that quantifies "ethical basketball" using real tracking data. EHI scores every player 0–100 based on how they earned their production — rewarding skill-based scoring and disciplined defense while penalizing manipulation, deception, and laziness.

> *"Ethical hoops" has been a growing conversation in the NBA for years. EHI is the first attempt to actually put a number on it.*

---

## What is EHI?

The **Ethical Hoops Index** is a composite metric made up of 5 sub-scores:

| Sub-Score | Weight | What it measures |
|---|---|---|
| **SQS** — Shot Quality Score | 35% | Did you take good shots or force bad ones? |
| **FDS** — Foul Drawing Score | 20% | Did you earn your free throws or manufacture them? |
| **FTP** — FT Dependency Score | 20% | What percentage of your points came from free throws? |
| **SPS** — Sportsmanship Score | 15% | Techs, flagrants, illegal screens, dirty play |
| **DES** — Defensive Effort Score | 10% | Contested shots, deflections, charges, active hands |

```
EHI = 0.35(SQS) + 0.20(FDS) + 0.20(FTP) + 0.15(SPS) + 0.10(DES)
```

All scores are **absolute** (0–100) and computed on a **per-game basis**.

---

## Motivation

The NBA has seen growing debate around "dark arts" basketball — foul hunting on 3-point attempts, off-ball contact manipulation, and late-game free throw padding. Traditional box score stats don't distinguish between a player who scored 30 points through elite shot-making and one who scored 30 through manufactured free throws.

EHI fills that gap.

---

## Key Design Decisions

- **Shot quality over efficiency** — bad shots that go in aren't penalized. Open misses aren't penalized. Only bad shots that miss are.
- **Foul legitimacy scoring** — each foul drawn gets a 0–1 legitimacy score based on defender distance, catch-and-shoot context, and repetition patterns within the game.
- **Garbage time detection** — FTA drawn in Q4 with a 20+ point lead and ≤5 minutes remaining are capped at 0.20 legitimacy regardless of context.
- **Exponential stacking** — repeated unethical acts in a single game compound (x^1.4), reflecting how fans and analysts perceive escalating bad behavior.
- **Volume awareness** — even legitimate free throw volume is penalized because ethical basketball means scoring through skill, not contact.

---

## Real World Stress Test: Bam Adebayo's 83-Point Game

On March 10, 2026, Bam Adebayo scored 83 points (2nd highest in NBA history) against the Washington Wizards — going 20-43 FG, 7-22 from 3, and an NBA-record 36-43 from the free throw line.

EHI breaks this game down honestly:
- Early scoring (31 pts in Q1) was legitimate and reflected in SQS
- 43 FTA triggers a massive FDS volume penalty
- FTP dependency ratio of ~43% tanks the FT Dependency score
- Garbage time free throw padding (14-16 FT in Q4 with 20+ lead) gets capped at 0.20 legitimacy

**This is exactly the kind of game EHI was designed to evaluate.**

---

## Tech Stack

| Layer | Tool |
|---|---|
| Data collection | Python + `nba_api` |
| Data processing | `pandas` |
| Storage | SQLite |
| Version control | Git / GitHub |

---

## Project Structure

```
EthicalHoopsIndex/
├── config.py                  # All tunable constants and NBA API headers
├── pipeline.py                # Data pipeline — pulls 6 endpoints from nba_api
├── compute_ehi.py             # EHI calculator — all 5 sub-scores
├── EHI_Master_Reference.md    # Full metric design document
├── CLAUDE.md                  # Project context for Claude Code sessions
├── .gitignore
└── LICENSE
```

---

## Data Pipeline

`pipeline.py` pulls 6 endpoints from `stats.nba.com` for any given game:

1. **LeagueGameFinder** — finds game ID by team and date
2. **BoxScoreSummaryV3** — game metadata
3. **BoxScoreTraditionalV3** — per-player box score (PTS, FGM, FTA, REB, STL, BLK, PF)
4. **PlayByPlayV3** — full play log for foul classification
5. **ShotChartDetail** — shot location, zone, made/missed, assisted/self-created
6. **BoxScoreHustleV2** — contested shots, deflections, charges drawn

---

## Sub-Score Status

- [x] FTP — FT Dependency Score
- [x] SPS — Sportsmanship Score  
- [x] DES — Defensive Effort Score
- [x] FDS — Foul Drawing Score
- [ ] SQS — Shot Quality Score *(in progress)*
- [ ] Full EHI aggregation
- [ ] Validation against 10–15 consensus games
- [ ] Dashboard / visualization layer

---

## Background

Built as a resume project by a Computer Engineering student at UGA, combining data engineering, sports analytics, and novel metric design. The project demonstrates a full data pipeline from API ingestion to composite metric calculation using real NBA tracking data.

---

## Disclaimer

This project is not affiliated with or endorsed by the NBA. Data is pulled from publicly available endpoints on `stats.nba.com` for educational and research purposes.

# Static planned-X shortlist on complete TERRY terrain
No live edits, scorer/timing runs, or original source/art used.
    
    g++ -std=c++23 geometry.cpp -o geometry && ./geometry > geometry.tsv
    g++ -std=c++23 edge.cpp -o edge && ./edge > edge.tsv
    python exposure.py edge.tsv ../ahc008_terry_exact_root/practice100.csv ../ahc008_focus_assets/tools/in ../ahc008_terry_scaffold_audit/tapes > exposure.json

Strict geometry fixes all room footprints, exact doors/areas/IDs, existing main-aisle reach, and bijectively preserves connected components on existing open cells, both with all gates open and virtually closed. Newly freed cells must belong to an excluded existing aisle component in the closed graph; no phantom site appears. Across all five completed maps every planned X directly touches a numbered room or door. No no-touch singleton/length2–4 straight group exists. Top+bottom X counts 6,10,8,9,11; merely being top/bottom does not produce a single compatible removal. Thus the strict shortlist has zero exposure on any tape.

One fallback considered because this set was empty: retire only the distinct nominal one-door physical corner rooms (room has tile at 0/29 × 0/29). Remove its door marker and only X which touches a retired room and nothing of any other room or its gates. Extra openings become aisle; all other numbered room sets, areas, doors and virtual-closed component relations (except intentionally retired sites) remain exactly the same. Retires 4/4/4/2/4 across maps 0..4, with only 2/2/2/1/2 exclusive X; those geometric identities pass even though actual capture potential changes.

All 100 original practice input/action digests and replayed scores verified. In the 98 factual full-clear tapes, opening those frozen X and actually closed retired gates leaks on 33 boards (skipping X alone leaks on 15). Raw retained mean area arithmetic +3.41 across all98 is not safe; certified +1.585 average across 65 still-safe cases, none reaches +10. No candidate recommended. Removing walls after the fact is not an allowed solver action and never establishes causal skip behavior.

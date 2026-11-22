"""Show paired topology only where both solvers separate all pets from humans."""
import argparse
import csv
from statistics import mean

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("scores")
    p.add_argument("topology")
    p.add_argument("baseline")
    p.add_argument("candidate")
    args=p.parse_args()
    data={(r["case"],r["model"]):r for r in csv.DictReader(open(args.scores))}
    top={(r["case"],r["model"]):r for r in csv.DictReader(open(args.topology))}
    keys=[i for i,k in data if k==args.baseline]
    all_clear=[i for i in keys
               if all(data[i,k]["status"]=="ok" and int(data[i,k]["all_humans_pet_free"])
                      for k in (args.baseline,args.candidate))]
    print(f"both all-clear: {len(all_clear)}/{len(keys)}")
    for field in ("distinct_safe_area","distinct_pet_only_area","largest_pet_only_area","human_components"):
        if all_clear:
            values=[mean(int(top[i,k][field]) for i in all_clear)
                    for k in (args.baseline,args.candidate)]
            print(field, *[round(x,3) for x in values])
        else:
            print(field,"no paired clear cases")
if __name__=="__main__":main()

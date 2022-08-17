import git, json
repo = git.Repo(search_parent_directories=True)
tags = repo.tags

with open("./flash_partitions/data/default", "r+") as f:
    data = f.read()
    data_js = json.loads(data)
    data_js["info"]["fw_version"] = str(tags[-1])


with open("./flash_partitions/data/default", "w") as f:
    f.write(json.dumps(data_js, indent=4))

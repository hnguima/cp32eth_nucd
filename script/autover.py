import git
repo = git.Repo(search_parent_directories=True)
tags = repo.tags

print("Current version:", tags)

#!/bin/bash

echo "Starting to convert all remote tracking branches to local branches..."
echo "-------------------------------------------------------------------"

# 获取所有远程跟踪分支的列表，并去除 "remotes/origin/" 前缀
# 排除 HEAD 引用，因为它不是一个实际的分支
remote_branches=$(git branch -r | grep 'origin/' | grep -v 'HEAD' | sed 's/.*origin\///')

if [ -z "$remote_branches" ]; then
    echo "No remote tracking branches found (e.g., remotes/origin/branch-name). Exiting."
    exit 0
fi

# 遍历每个分支并创建本地分支
for branch in $remote_branches; do
    # 检查本地是否已经存在同名分支，避免重复创建报错
    if git rev-parse --verify --quiet "$branch" >/dev/null; then
        echo "Local branch '$branch' already exists. Skipping."
    else
        echo "Creating local branch: $branch based on origin/$branch"
        # 使用 git branch <new-branch> <start-point> 来创建本地分支
        # 这里 start-point 就是对应的远程跟踪分支
        git branch "$branch" "origin/$branch"
        if [ $? -ne 0 ]; then
            echo "Error creating branch '$branch'. Please check the output above."
        fi
    fi
done

echo "-------------------------------------------------------------------"
echo "Finished converting remote tracking branches."
echo "Here are your current local branches:"
git branch

echo ""
echo "You can now switch to any of these local branches using 'git checkout <branch-name>'."

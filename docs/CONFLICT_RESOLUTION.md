# Resolving Merge Conflicts for UrDesign

This guide walks you through fixing merge conflicts when updating a branch or
pull request. You do not need to be a Git expert—just follow the steps in the
order they are listed. Wherever you see text in `<angle brackets>`, replace it
with information that matches your situation.

## 1. Update your local repository

1. Open a terminal inside your UrDesign checkout.
2. Make sure you are on the branch that backs your pull request:
   ```bash
   git status
   ```
   The output should mention `On branch <your-branch-name>`.
3. Fetch the newest changes from the main repository and update your local
   copy of the target branch (usually `main`):
   ```bash
   git fetch origin
   git checkout main
   git pull origin main
   ```

## 2. Rebase or merge the target branch

You have two options. Rebase keeps history linear, while merge preserves
existing commits. Pick the approach your collaborators prefer.

### Option A – Rebase (recommended for clean history)

1. Return to your feature branch:
   ```bash
   git checkout <your-branch-name>
   ```
2. Replay your commits on top of the latest `main` branch:
   ```bash
   git rebase main
   ```
3. If Git reports conflicts, note the files that need attention and continue
   with [Section 3](#3-fix-conflicted-files).
4. Once every conflict is fixed, resume the rebase:
   ```bash
   git rebase --continue
   ```

### Option B – Merge (when rebasing is discouraged)

1. Stay on your feature branch and merge the target branch into it:
   ```bash
   git merge main
   ```
2. Git stops and reports conflicts. Continue with [Section 3](#3-fix-conflicted-files).
3. After fixing conflicts, complete the merge with:
   ```bash
   git commit
   ```

## 3. Fix conflicted files

1. Open each file Git lists as conflicted. Conflict blocks look like this:
   ```
   <<<<<<< HEAD
   your current changes
   =======
   incoming changes from main
   >>>>>>> origin/main
   ```
2. Read both versions before deciding what stays. The lines between `<<<<<<<`
   and `=======` show what currently exists on **your branch**. The lines
   between `=======` and `>>>>>>>` show what comes from the **other branch** you
   are merging. If the block is long, run the following command to compare the
   two versions in a temporary view without editing anything yet:
   ```bash
   git diff --color-words --ours --theirs -- <file-name>
   ```
   This prints the differences between your branch (`--ours`) and the incoming
   branch (`--theirs`) so you can read both in plain language.
3. Decide what the final text should be. You can keep either side or combine
   them—just remove the `<<<<<<<`, `=======`, and `>>>>>>>` markers afterwards.
3. Save the file once it shows the intended content.
4. When all conflicts are resolved, stage the files:
   ```bash
   git add <file1> <file2> ...
   ```

## 4. Verify and continue

1. Ask Git whether any conflicts remain:
   ```bash
   git status
   ```
   If the status reports “All conflicts fixed but you are still merging,” you
   can continue with the rebase or merge.
2. For a rebase, run:
   ```bash
   git rebase --continue
   ```
   Repeat the conflict resolution steps until Git finishes rebasing.
3. For a merge, finish by creating the merge commit:
   ```bash
   git commit
   ```

## 5. Run tests (optional but encouraged)

If the project has automated tests or build steps, run them now to ensure your
combined changes behave correctly.

## 6. Push the updated branch

Finally, publish the resolved history to the remote branch that backs your pull
request:
```bash
git push --force-with-lease
```
Use `--force-with-lease` after a rebase because the commit hashes changed. For a
merge, a regular `git push` is enough.

## 7. Confirm in your pull request

Visit your pull request in the browser. GitHub should now show that the branch
has no conflicts with the target branch. Leave a short comment summarizing what
you did so reviewers know the conflicts were handled.

---

Whenever new conflicts arise, repeat these steps. Over time you will get faster
at spotting what needs to be kept from each side, but the overall workflow
remains the same.

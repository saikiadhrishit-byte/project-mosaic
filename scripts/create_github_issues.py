#!/usr/bin/env python3
import os
import sys
import json
import urllib.request
import urllib.error
import subprocess
import re

# Curated color palette for labels
LABEL_COLORS = {
    "good first issue": "7057ff",
    "help wanted": "008672",
    "architecture": "d4c5f9",
    "documentation": "0075ca",
    "cpp": "f18e33",
    "compiler": "1d76db",
    "IR": "c5def5",
    "graph": "bfd4f2",
    "algorithms": "5319e7",
    "vulkan": "a2eeef",
    "graphics": "e99695",
    "2d": "f9d0c4",
    "testing": "c2e0c6",
    "research": "fef2c0"
}

def get_git_remote_repo():
    try:
        url = subprocess.check_output(["git", "config", "--get", "remote.origin.url"], text=True).strip()
        # Parse owner/repo from URL
        # Matches: git@github.com:owner/repo.git or https://github.com/owner/repo.git
        match = re.search(r"github\.com[:/]([^/]+/[^/.]+)(?:\.git)?", url)
        if match:
            return match.group(1)
    except Exception:
        pass
    return "saikiadhrishit-byte/project-nysor"

WAS_TOKEN_ERROR = False

def github_request(url, token, data=None, method="GET"):
    global WAS_TOKEN_ERROR
    headers = {
        "Authorization": f"token {token}",
        "Accept": "application/vnd.github.v3+json",
        "User-Agent": "Project-Nysor-Issue-Creator",
        "Content-Type": "application/json"
    }
    
    body = json.dumps(data).encode("utf-8") if data is not None else None
    req = urllib.request.Request(url, data=body, headers=headers, method=method)
    
    try:
        with urllib.request.urlopen(req) as response:
            return json.loads(response.read().decode("utf-8")), response.status
    except urllib.error.HTTPError as e:
        # For label creation, a 422 usually means the label already exists
        err_body = e.read().decode("utf-8")
        try:
            err_json = json.loads(err_body)
        except Exception:
            err_json = {"message": err_body}
            
        msg = err_json.get("message", "")
        if "resource not accessible" in msg.lower() or (e.code in (403, 404) and "accessible" in msg.lower()):
            WAS_TOKEN_ERROR = True
            
        return err_json, e.code
    except Exception as e:
        print(f"Error making request to {url}: {e}", file=sys.stderr)
        return None, 500

def create_labels(repo, token, labels, dry_run=False):
    print("Checking and creating labels...")
    for label in labels:
        color = LABEL_COLORS.get(label, "cfd3d7") # default grey if not in color map
        url = f"https://api.github.com/repos/{repo}/labels"
        
        if dry_run:
            print(f"[DRY-RUN] Create label: '{label}' with color #{color}")
            continue
            
        payload = {
            "name": label,
            "color": color,
            "description": f"Nysor {label} topic"
        }
        res, status = github_request(url, token, payload, "POST")
        if status == 201:
            print(f"Successfully created label: '{label}'")
        elif status == 422:
            print(f"Label '{label}' already exists or failed to validate.")
        else:
            print(f"Error creating label '{label}': {res.get('message', 'Unknown error')}")

def create_issues(repo, token, issues, dry_run=False):
    print("\nCreating issues...")
    for issue in issues:
        title = issue.get("title")
        body = issue.get("body")
        labels = issue.get("labels", [])
        
        url = f"https://api.github.com/repos/{repo}/issues"
        
        if dry_run:
            print(f"\n[DRY-RUN] Create issue #{issue['number']}:")
            print(f"  Title: {title}")
            print(f"  Labels: {', '.join(labels)}")
            print("  Body snippet:")
            lines = body.split("\n")
            for line in lines[:4]:
                print(f"    {line}")
            if len(lines) > 4:
                print("    ...")
            continue
            
        payload = {
            "title": title,
            "body": body,
            "labels": labels
        }
        res, status = github_request(url, token, payload, "POST")
        if status == 201:
            print(f"Successfully created Issue #{issue['number']}: \"{title}\" -> {res.get('html_url')}")
        else:
            error_msg = res.get('message', 'Unknown error') if res else 'No response'
            print(f"Failed to create Issue #{issue['number']}: \"{title}\". Error: {error_msg}")

def close_existing_issues(repo, token, dry_run=False):
    print("\nChecking for existing open issues to close...")
    url = f"https://api.github.com/repos/{repo}/issues?state=open"
    res, status = github_request(url, token)
    if status != 200:
        # If we got a dict or list, handle appropriately
        msg = res.get('message', 'Unknown error') if isinstance(res, dict) else 'Unknown error'
        print(f"Warning: Could not fetch existing issues: {msg}")
        return
        
    if not isinstance(res, list):
        print("Warning: Received invalid response structure when fetching issues.")
        return

    # Filter out pull requests (issues API returns both issues and PRs)
    open_issues = [item for item in res if "pull_request" not in item]
    
    if not open_issues:
        print("No open issues found.")
        return
        
    print(f"Found {len(open_issues)} open issues. Closing them...")
    for issue in open_issues:
        num = issue["number"]
        title = issue["title"]
        patch_url = f"https://api.github.com/repos/{repo}/issues/{num}"
        
        if dry_run:
            print(f"[DRY-RUN] Close issue #{num}: \"{title}\"")
            continue
            
        payload = {"state": "closed", "state_reason": "not_planned"}
        patch_res, patch_status = github_request(patch_url, token, payload, "PATCH")
        if patch_status == 200:
            print(f"Successfully closed issue #{num}: \"{title}\"")
        else:
            msg = patch_res.get('message', 'Unknown error') if patch_res else 'Unknown error'
            print(f"Failed to close issue #{num}: {msg}")

def main():
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    if hasattr(sys.stderr, 'reconfigure'):
        sys.stderr.reconfigure(encoding='utf-8')

    import argparse
    parser = argparse.ArgumentParser(description="Create Project Nysor GitHub issues and labels.")
    parser.add_argument("--dry-run", action="store_true", help="Print actions without modifying GitHub.")
    parser.add_argument("--recreate", action="store_true", help="Close existing open issues before creating new ones.")
    args = parser.parse_args()

    # Locate issues json
    script_dir = os.path.dirname(os.path.abspath(__file__))
    issues_path = os.path.join(script_dir, "..", ".github", "issues", "issue_list.json")
    if not os.path.exists(issues_path):
        print(f"Error: Could not find issue list at {issues_path}", file=sys.stderr)
        sys.exit(1)
        
    with open(issues_path, "r", encoding="utf-8-sig") as f:
        issues = json.load(f)
        
    repo = get_git_remote_repo()
    print(f"Target repository: {repo}")
    
    if args.dry_run:
        print("Running in DRY-RUN mode. No changes will be made to GitHub.")
        token = "mock-token"
    else:
        token = os.environ.get("GITHUB_TOKEN")
        if not token:
            print("GitHub Personal Access Token (PAT) not found in GITHUB_TOKEN environment variable.")
            token = input("Please enter your GitHub PAT: ").strip()
            if not token:
                print("Error: GitHub PAT is required to publish issues.")
                sys.exit(1)
                
    # Gather all unique labels
    unique_labels = set()
    for issue in issues:
        unique_labels.update(issue.get("labels", []))
        
    create_labels(repo, token, sorted(unique_labels), dry_run=args.dry_run)
    if args.recreate:
        close_existing_issues(repo, token, dry_run=args.dry_run)
    create_issues(repo, token, issues, dry_run=args.dry_run)
    
    if WAS_TOKEN_ERROR:
        print("\n" + "="*70)
        print("API ACCESS ERROR: Resource not accessible by personal access token.")
        print(f"\nThis means your GitHub Personal Access Token (PAT) does not have")
        print(f"permission to write to the repository '{repo}'.")
        print("\nIf you are using a Fine-Grained Token:")
        print("1. Go to: Settings -> Developer Settings -> Personal Access Tokens -> Fine-grained tokens")
        print("2. Edit your token and ensure 'Repository access' is set to 'All repositories'")
        print("   or explicitly select 'project-nysor' under 'Only select repositories'.")
        print("3. Under 'Repository permissions', set 'Issues' to 'Read and write'.")
        print("4. Save/update the token and run this script again.")
        print("\nIf you are using a Classic Token:")
        print("1. Go to: Settings -> Developer Settings -> Personal Access Tokens -> Tokens (classic)")
        print("2. Edit your token and ensure the 'repo' scope (or 'public_repo') is checked.")
        print("3. Save/update the token and run this script again.")
        print("="*70 + "\n")
        sys.exit(1)
        
    print("\nDone!")

if __name__ == "__main__":
    main()

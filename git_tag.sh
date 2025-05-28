#!/bin/bash

set -e

ProductName=utils
Product_version_key="TheLibVersion"
REPO_PFEX=zefrain/${ProductName}
VersionFile=VERSION
readonly REMAIN_VERSION=3 # 保留最新的版本数

CURRENT_VERSION=$(grep ${Product_version_key} $VersionFile | awk -F '"' '{print $2}' | sed 's/\"//g')

NEXT_VERSION=""

OSTYPE="Unknown"
GetOSType() {
  uNames=$(uname -s)
  osName=${uNames:0:4}
  if [ "$osName" == "Darw" ]; then # Darwin
    OSTYPE="Darwin"
  elif [ "$osName" == "Linu" ]; then # Linux
    OSTYPE="Linux"
  elif [ "$osName" == "MING" ]; then # MINGW, windows, git-bash
    OSTYPE="Windows"
  else
    OSTYPE="Unknown"
  fi
}
GetOSType

function install_dependencies() {
  if [ "$OSTYPE" == "Darwin" ]; then
    if ! command -v brew &>/dev/null; then
      echo "Homebrew is not installed. Please install it from https://brew.sh/."
      exit 1
    fi
    brew install gh || true
  elif [ "$OSTYPE" == "Linux" ]; then

    source /etc/os-release

    if [[ "$ID" == "ubuntu" || "$ID" == "debian" ]]; then
      sudo apt-get update
      sudo apt-get install -y gh
    elif [[ "$ID" == "fedora" || "$ID" == "centos" ]]; then
      sudo dnf install -y gh || sudo yum install -y gh
    elif [[ "$ID" == "arch" ]]; then
      sudo pacman -S --noconfirm gh
    else
      echo "Unsupported Linux distribution: $ID"
      exit 1
    fi

    if ! command -v gh &>/dev/null; then
      echo "gh CLI is not installed. Please install it using your package manager."
      exit 1
    fi
  elif [ "$OSTYPE" == "Windows" ]; then
    echo "Please install GitHub CLI from https://cli.github.com/."
    exit 1
  else
    echo "Unsupported OS type: $OSTYPE"
    exit 1
  fi
}

function setup_gh() {
  if ! command -v gh &>/dev/null; then
    echo "gh CLI is not installed. Please install it from https://cli.github.com/."
    exit 1
  fi

  if ! gh auth status &>/dev/null; then
    echo "You need to authenticate with GitHub CLI."
    gh auth login
  fi
  if ! gh repo view $REPO_PFEX &>/dev/null; then
    echo "Repository $REPO_PFEX does not exist or you do not have access."
    exit 1
  fi
}

function to_run() {
  if [ -z "$1" ]; then
    baseStr=$(echo ${CURRENT_VERSION} | cut -d'.' -f1) # Get the base version (v0)
    base=${baseStr//v/}                                # Get the base version (0)
    major=$(echo ${CURRENT_VERSION} | cut -d'.' -f2)   # Get the major version (0)
    minor=$(echo ${CURRENT_VERSION} | cut -d'.' -f3)   # Get the minor version (1)

    minor=$((minor + 1))       # Increment the minor version
    if ((minor == 1000)); then # Check if minor version is 1000
      minor=0                  # Reset minor version to 0
      major=$((major + 1))     # Increment major version
    fi

    if ((major == 1000)); then # Check if major version is 1000
      major=0                  # Reset major version to 0
      base=$((base + 1))       # Increment base version
    fi

    NEXT_VERSION="v${base}.${major}.${minor}"
    return 0
  elif [ "$1" == "custom" ]; then
    echo "============================ ${ProductName} ============================"
    echo "  1、发布 [-${ProductName}-]"
    echo "  当前版本[-${CURRENT_VERSION}-]"
    echo "======================================================================"
    read -p "$(echo -e "请输入版本号[例如；v0.0.1]")" inputString
    if [[ "$inputString" =~ ^v.* ]]; then
      NEXT_VERSION=${inputString}
    else
      NEXT_VERSION=v${inputString}
    fi
    return 0
  else
    return 1
  fi
}

function get_pre_del_version_no {
  local v_str=$1
  baseStr=$(echo $v_str | cut -d'.' -f1) # Get the base version (v0)
  base=${baseStr//v/}                    # Get the base version (0)
  major=$(echo $v_str | cut -d'.' -f2)   # Get the major version (0)
  minor=$(echo $v_str | cut -d'.' -f3)   # Get the minor version (1)

  if ((minor > 0)); then # Check if minor version is more than 0
    minor=$((minor - 1)) # Decrement the minor version
  else
    minor=999              # Reset minor version to 999
    if ((major > 0)); then # Check if major version is more than 0
      major=$((major - 1)) # Decrement major version
    else
      major=999             # Reset major version to 999
      if ((base > 0)); then # Check if base version is more than 0
        base=$((base - 1))  # Decrement base version
      else
        echo "Error: Version cannot be decremented."
        exit 1
      fi
    fi
  fi

  pre_v_no="${base}.${major}.${minor}"
  echo $pre_v_no
}

function git_handle_ready() {
  echo "Current Version With "${CURRENT_VERSION}
  echo "Next Version With "${NEXT_VERSION}

  sed -i -e "s/\(${Product_version_key}[[:space:]]*\"\)${CURRENT_VERSION}\"/\1${NEXT_VERSION}\"/" $VersionFile

  if [[ $OSTYPE == "Darwin" ]]; then
    echo "rm darwin cache"
    rm -rf $VersionFile"-e"
  fi
}

function git_handle_push() {
  local current_version_no=${CURRENT_VERSION//v/}
  local netx_version_no=${NEXT_VERSION//v/}

  # 保留前3个最新 tag（按版本号降序）
  TAGS_TO_DELETE=$(git tag --sort=-v:refname | tail -n +${REMAIN_VERSION})
  echo "Tags to delete: $TAGS_TO_DELETE"

  for TAG in $TAGS_TO_DELETE; do
    echo "Deleting tag: $TAG"

    # 删除本地 tag
    git tag -d "$TAG"

    # 删除远程 tag
    git push origin ":refs/tags/$TAG"

    # 删除 GitHub release（需要 gh CLI）
    gh release delete "$TAG" --yes || echo "Release $TAG not found or already deleted"
  done

  # git add . \
  git add -u &&
    git commit -m "Update v${netx_version_no}" &&
    git tag v$netx_version_no &&
    git tag -f latest v$netx_version_no &&
    git push --tags -f

}

handle_input() {
  if [[ $1 == "-get_pre_del_tag_name" ]]; then
    pre_tag=$(get_pre_del_version_no "${CURRENT_VERSION}")
    echo $pre_tag
  elif [ -z "$1" ] || [ "$1" == "auto" ]; then

    if to_run "$1"; then
      git_handle_ready
      git_handle_push
      echo "Complated"
    else
      echo "Invalid argument normal"
    fi
  else
    echo "Invalid argument"
  fi
}

handle_input "$@"

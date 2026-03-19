# Quick Start Guide

## 3 Steps to Build

### Step 1: Create GitHub Repository
1. Go to https://github.com/new
2. Enter repository name: `SoulBypass`
3. Select **Public**
4. Click **Create repository**

### Step 2: Upload Files
1. In your new repository, click **"uploading an existing file"**
2. Drag and drop ALL files from the extracted zip
   - MUST include: `.github/`, `module/`, `zygisk/`, `build.sh`, `customize.sh`, `module.prop`, `README.md`
3. Click **Commit changes**

### Step 3: Wait for Build
1. Click **Actions** tab at the top
2. Wait 2-3 minutes for green checkmark ✅
3. Click the latest run
4. Download **SoulBypass-Module** from Artifacts section

## Installation

1. Extract the downloaded zip to get `SoulBypass-v1.0.0.zip`
2. Transfer to phone
3. Magisk → Modules → Install from storage
4. Reboot

## Troubleshooting

**If build fails:**
- Check that all folders are uploaded (especially `zygisk/` and `module/`)
- Check Actions log for specific errors

**If module doesn't work:**
- Ensure Zygisk is enabled in Magisk
- Add Soul App to Magisk Hide list
- Check logs: `adb logcat -s SoulBypass`

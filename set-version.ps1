<#
.SYNOPSIS
    Bumps the app version, commits pending changes, creates an annotated git
    tag, and pushes — which triggers the GitHub Actions build + release.

.DESCRIPTION
    Keeps version strings in sync across:
      - NTFSfastFind/NTFSfastFind.cpp   (#define _VERSION "vX.Y.Z")
      - README.md                       (<!--version--> and <!--date--> markers)
      - VERSION                         (X.Y.Z, no leading v)

    Then: git add → git commit → git tag -a vX.Y.Z → git push --follow-tags.

.PARAMETER Version
    Version string, with or without leading 'v'. e.g.  3.02  or  v3.02  or  1.2.3

.PARAMETER Message
    Commit + annotated tag message.

.EXAMPLE
    .\set-version.ps1 -Version 3.03 -Message "fix: long-path filename extraction"
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory, Position = 0)]
    [ValidatePattern('^v?\d+\.\d+(\.\d+(\.\d+)?)?$')]
    [string]$Version,

    # ValueFromRemainingArguments lets the user pass the message unquoted or
    # with terminal-mangled quotes (smart quotes etc). All trailing tokens are
    # joined back into a single string.
    [Parameter(Mandatory, Position = 1, ValueFromRemainingArguments=$true)]
    [string[]]$MessageParts
)

$Message = ($MessageParts -join ' ').Trim()
if ([string]::IsNullOrWhiteSpace($Message)) {
    Write-Error "Message is required."
    exit 1
}

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# Resolve repo root from script location so paths work regardless of cwd.
$root = $PSScriptRoot
Set-Location $root

# Normalize: $Version = '3.02'  (no v),  $Tag = 'v3.02'
$ver = $Version -replace '^v',''
$tag = "v$ver"
$date = Get-Date -Format 'MMM dd yyyy'   # matches C __DATE__ shape
$year = (Get-Date).Year

# Win32 VERSIONINFO needs a 4-part numeric tuple. Split, cast to int, pad.
$parts = $ver.Split('.') | ForEach-Object { [int]$_ }
while ($parts.Count -lt 4) { $parts += 0 }
$winTuple = ($parts[0..3] -join ',')      # "3,2,0,0"
$winDots  = ($parts[0..3] -join '.')      # "3.2.0.0"

Write-Host "Repo    : $root"
Write-Host "Version : $ver  ->  tag $tag"
Write-Host "Date    : $date"
Write-Host ""

# --- Sanity: must be in a git repo, on a branch, no existing tag ------------
git rev-parse --show-toplevel 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) { Write-Error "Not inside a git repository."; exit 1 }

if ((git tag -l $tag) -eq $tag) {
    Write-Error "Tag '$tag' already exists. Delete it first: git tag -d $tag"
    exit 1
}

# --- 1. NTFSfastFind.cpp:   #define _VERSION "vX.Y.Z" ----------------------
$cpp = Join-Path $root 'NTFSfastFind\NTFSfastFind.cpp'
$cppText = [System.IO.File]::ReadAllText($cpp)
if ($cppText -notmatch '#define\s+_VERSION\s+"v[^"]*"') {
    Write-Error "_VERSION define not found in $cpp"
    exit 1
}
$cppText = $cppText -replace '(#define\s+_VERSION\s+")v[^"]*(")', "`${1}$tag`${2}"
[System.IO.File]::WriteAllText($cpp, $cppText)
Write-Host "Updated : NTFSfastFind\NTFSfastFind.cpp   _VERSION = `"$tag`""

# --- 2. README.md:  <!--version-->vX.Y.Z   and   <!--date-->Mmm DD YYYY -----
$readme = Join-Path $root 'README.md'
$readmeText = [System.IO.File]::ReadAllText($readme)
$readmeText = $readmeText -replace '(<!--\s*version\s*-->)v[^\s<]+', "`${1}$tag"
$readmeText = $readmeText -replace '(<!--\s*date\s*-->)[A-Za-z]+\s+\d+\s+\d{4}', "`${1}$date"
[System.IO.File]::WriteAllText($readme, $readmeText)
Write-Host "Updated : README.md                       version=$tag  date=$date"

# --- 3. VERSION file (numeric, no v) ---------------------------------------
# Use platform-native line ending (CRLF on Windows) so `git add` doesn't emit
# the "LF will be replaced by CRLF" warning that ErrorActionPreference=Stop
# would otherwise turn into a fatal error.
$verFile = Join-Path $root 'VERSION'
[System.IO.File]::WriteAllText($verFile, "$ver$([Environment]::NewLine)")
Write-Host "Updated : VERSION                         $ver"

# --- 4. NTFSfastFind.rc:  Win32 VERSIONINFO (UTF-16 LE) --------------------
# Drives the Windows Explorer Details tab: FileVersion / ProductVersion /
# LegalCopyright. Encoding must stay UTF-16 LE with BOM.
$rc = Join-Path $root 'NTFSfastFind\NTFSfastFind.rc'
$rcText = [System.IO.File]::ReadAllText($rc, [System.Text.Encoding]::Unicode)
$rcText = $rcText -replace 'FILEVERSION\s+\d+,\d+,\d+,\d+',                          "FILEVERSION    $winTuple"
$rcText = $rcText -replace 'PRODUCTVERSION\s+\d+,\d+,\d+,\d+',                       "PRODUCTVERSION $winTuple"
$rcText = $rcText -replace '(VALUE\s+"FileVersion",\s+)"[^"]+"',                     "`${1}`"$winDots`""
$rcText = $rcText -replace '(VALUE\s+"ProductVersion",\s+)"[^"]+"',                  "`${1}`"$winDots`""
$rcText = $rcText -replace '(VALUE\s+"LegalCopyright",\s+"Copyright \(C\) )\d+',     "`${1}$year"
[System.IO.File]::WriteAllText($rc, $rcText, [System.Text.Encoding]::Unicode)
Write-Host "Updated : NTFSfastFind\NTFSfastFind.rc     $winDots  (c) $year"

# --- 5. Commit, tag, push ---------------------------------------------------
# Helper: run git, surface output, fail only on non-zero exit code.
# Without `2>&1`, stderr from git (warnings) gets wrapped as a PowerShell
# ErrorRecord and ErrorActionPreference=Stop kills the script.
function Invoke-Git {
    param([Parameter(ValueFromRemainingArguments=$true)][string[]]$GitArgs)
    & git @GitArgs 2>&1 | ForEach-Object { Write-Host $_ }
    if ($LASTEXITCODE -ne 0) {
        Write-Error ("git " + ($GitArgs -join ' ') + " failed (exit $LASTEXITCODE)")
        exit 1
    }
}

Write-Host ""
Invoke-Git add NTFSfastFind/NTFSfastFind.cpp NTFSfastFind/NTFSfastFind.rc README.md VERSION
Invoke-Git commit -m $Message
Invoke-Git tag -a $tag -m $Message

Write-Host "Tagged  : $tag"
# Push branch and tag as SEPARATE operations so GitHub delivers two webhook
# events. With `--follow-tags` (one push), GitHub may coalesce them and skip
# the tag-triggered workflow run that the release job needs.
Write-Host "Pushing : branch -> origin"
Invoke-Git push origin HEAD
Write-Host "Pushing : tag $tag -> origin"
Invoke-Git push origin $tag

Write-Host ""
Write-Host "Done. Pushed $tag - GitHub Actions build + release should now run."

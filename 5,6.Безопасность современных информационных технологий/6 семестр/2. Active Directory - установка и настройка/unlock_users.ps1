param(
    [Parameter(Mandatory=$true)]
    [string]$OUName
)

Import-Module ActiveDirectory

$SearchBase = "OU=$OUName,DC=sh,DC=com"

try {
    $UserList = Search-ADAccount -SearchBase $SearchBase -LockedOut -UsersOnly -ErrorAction Stop
}
catch {
    Write-Host "ERROR: failed to search locked users in $SearchBase"
    Write-Host $_.Exception.Message
    exit 1
}

if(-not $UserList)
{
    Write-Host "[Unlocker] All users in this organizational unit are unlocked."
    exit 0
}

foreach($User in $UserList)
{
    try {
        Unlock-ADAccount -Identity $User.SamAccountName -ErrorAction Stop
        Write-Host "[Unlocker] $($User.SamAccountName) was unlocked!"
    }
    catch {
        Write-Host "[Unlocker] FAILED to unlock $($User.SamAccountName)"
        Write-Host $_.Exception.Message
    }
}
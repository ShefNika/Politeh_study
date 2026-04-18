Import-Module ActiveDirectory

$domainL1 = "com"
$domainL2 = "sh"
$UserOU = "Others"
$FileServerPath = "\\SERVER\Personal"

if (-not $args[0]) {
    Write-Host "Usage: .\create_user.ps1 <username>"
    exit 1
}

$UserName = $args[0]
$UserPath = "OU=$UserOU,DC=$domainL2,DC=$domainL1"

try {
    New-ADUser `
        -Name $UserName `
        -GivenName $UserName `
        -SamAccountName $UserName `
        -Path $UserPath `
        -AccountPassword (Read-Host -AsSecureString "New user pass") `
        -Enabled $true `
        -PasswordNeverExpires $false `
        -ErrorAction Stop

    Write-Host "User $UserName was created in AD!"
}
catch {
    Write-Host "ERROR: failed to create user $UserName in AD."
    Write-Host $_.Exception.Message
    exit 1
}

try {
    $UserOwner = Get-ADUser -Identity $UserName -ErrorAction Stop
}
catch {
    Write-Host "ERROR: user $UserName was not found in AD after creation."
    exit 1
}

$NewFolderPath = Join-Path $FileServerPath $UserName

if (-not (Test-Path $NewFolderPath)) {
    try {
        New-Item -ItemType Directory -Path $FileServerPath -Name $UserName -ErrorAction Stop | Out-Null
        Write-Host "Folder $NewFolderPath was created."
    }
    catch {
        Write-Host "ERROR: failed to create folder $NewFolderPath."
        Write-Host $_.Exception.Message
        exit 1
    }
}
else {
    Write-Host "Folder $NewFolderPath already exists."
}

try {
    $Acl = Get-Acl $NewFolderPath -ErrorAction Stop
    $Ar = New-Object System.Security.AccessControl.FileSystemAccessRule($UserOwner.SID, "FullControl", "Allow")
    $Acl.SetAccessRule($Ar)
}
catch {
    Write-Host "ERROR: failed to prepare ACL for $NewFolderPath."
    Write-Host $_.Exception.Message
    exit 1
}

$OUAll = Get-ADOrganizationalUnit -SearchBase "DC=$domainL2,DC=$domainL1" -SearchScope Subtree -Filter *

foreach ($ou in $OUAll) {
    if ($ou.Name -eq $UserOU) {
        Get-ADUser -Filter * -SearchBase $ou | ForEach-Object {
            if ($_.Name -ne $UserOwner.Name) {
                try {
                    $Ar = New-Object System.Security.AccessControl.FileSystemAccessRule($_.SID, "FullControl", "Deny")
                    $Acl.SetAccessRule($Ar)

                    $OldUserFolder = Join-Path $FileServerPath $_.Name
                    if (Test-Path $OldUserFolder) {
                        $AclTmp = Get-Acl $OldUserFolder
                        $ArTmp = New-Object System.Security.AccessControl.FileSystemAccessRule($UserOwner.SID, "FullControl", "Deny")
                        $AclTmp.SetAccessRule($ArTmp)
                        Set-Acl $OldUserFolder $AclTmp
                    }
                }
                catch {
                    Write-Host "WARNING: failed to update ACL for $($_.Name)"
                }
            }
        }
    }
}

try {
    Set-Acl $NewFolderPath $Acl -ErrorAction Stop
    Write-Host "ACL for $NewFolderPath was updated."
}
catch {
    Write-Host "ERROR: failed to apply ACL to $NewFolderPath."
    Write-Host $_.Exception.Message
    exit 1
}
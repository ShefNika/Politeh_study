Import-Module ActiveDirectory

$domainL1="com"
$domainL2="sh"
$domainL3="Admins"


New-ADUser -Name "UserAccManager" -SamAccountName "UserAccManager" -Path "OU=$domainL3, DC=$domainL2, DC=$domainL1" -AccountPassword(ConvertTo-SecureString "1234AcM" -AsPlainText -Force) -Enable $true
New-ADUser -Name "UserHelpDesk" -SamAccountName "UserHelpDesk" -Path "OU=$domainL3, DC=$domainL2, DC=$domainL1" -AccountPassword(ConvertTo-SecureString "1234HeD" -AsPlainText -Force) -Enable $true
New-ADUser -Name "UserGenAdmin" -SamAccountName "UserGenAdmin" -Path "OU=$domainL3, DC=$domainL2, DC=$domainL1" -AccountPassword(ConvertTo-SecureString "1234GeA" -AsPlainText -Force) -Enable $true
New-ADUser -Name "UserResAdmin" -SamAccountName "UserResAdmin" -Path "OU=$domainL3, DC=$domainL2, DC=$domainL1" -AccountPassword(ConvertTo-SecureString "1234ReA" -AsPlainText -Force) -Enable $true

Add-ADGroupMember "AccountManagers" UserAccManager
Add-ADGroupMember "HelpDesk" UserHelpDesk
Add-ADGroupMember "GeneralAdmins" UserGenAdmin
Add-ADGroupMember "ResourceAdmins" UserResAdmin
Import-Module ActiveDirectory

$name_ou = "Admins"
$name_dc = "sh"
$name_dc_com = "com"

New-ADGroup -Name "GeneralAdmins" -GroupCategory Security -groupScope Global -Path "ou=$name_ou,dc=$name_dc,dc=$name_dc_com"

New-ADGroup -Name "AccountManagers" -GroupCategory Security -groupScope Global -ManagedBy "GeneralAdmins" -Path "ou=$name_ou,dc=$name_dc,dc=$name_dc_com"

New-ADGroup -Name "HelpDesk" -GroupCategory Security -groupScope Global -ManagedBy "GeneralAdmins" -Path "ou=$name_ou,dc=$name_dc,dc=$name_dc_com"

New-ADGroup -Name "ResourceAdmins" -GroupCategory Security -groupScope Global -ManagedBy "GeneralAdmins" -Path "ou=$name_ou,dc=$name_dc,dc=$name_dc_com"
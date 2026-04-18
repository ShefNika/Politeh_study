dsacls.exe "DC=sh,DC=com" /I:T /G "sh\AccountManagers:CCDC;user"
dsacls.exe "DC=sh,DC=com" /I:S /G "sh\AccountManagers:RCWDWO;;user"
dsacls.exe "DC=sh,DC=com" /I:S /G "sh\AccountManagers:CALO;;user"
dsacls.exe "DC=sh,DC=com" /I:S /G "sh\AccountManagers:RPWP;userAccountControl;user"
dsacls.exe "DC=sh,DC=com" /I:S /G "sh\AccountManagers:RPWP;pwdLastSet;user"
dsacls.exe "DC=sh,DC=com" /I:S /G "sh\HelpDesk:RPWP;lockoutTime;user"

dsmod group "CN=Administrators,CN=Builtin,DC=sh,DC=com" -addmbr "CN=GeneralAdmins,OU=Admins,DC=sh,DC=com"
dsmod group "CN=IIS_IUSRS,CN=Builtin,DC=sh,DC=com" -addmbr "CN=ResourceAdmins,OU=Admins,DC=sh,DC=com"
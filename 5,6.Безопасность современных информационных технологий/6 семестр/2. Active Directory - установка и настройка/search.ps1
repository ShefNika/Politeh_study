param([int]$number = 5)

Import-Module ActiveDirectory

$users_expire_date = Get-ADUser -Filter {Enabled -eq $True -and PasswordNeverExpires -eq $False} -Properties "SamAccountName", "msDS-UserPasswordExpiryTimeComputed", "PasswordLastSet" |
Select-Object -Property "SamAccountName", @{Name="ExpiryDate"; Expression={[datetime]::FromFileTime($_."msDS-UserPasswordExpiryTimeComputed")}}

foreach($item in $users_expire_date)
{
    $expire_date = $item.ExpiryDate
    $current_date = Get-Date
    $DisplayName = $item.SamAccountName

    if($current_date -gt $item.ExpiryDate)
    {
        Write-Host "The password of user $DisplayName has expired!"
        continue
    }

    $lasts = $expire_date - $current_date

    if($lasts.Days -lt $number)
    {
        Write-Host "The password of user $DisplayName expires less than $number day(s)!"
        Write-Host "The password of user $DisplayName expires: $expire_date"
        Write-Host ""
    }
}
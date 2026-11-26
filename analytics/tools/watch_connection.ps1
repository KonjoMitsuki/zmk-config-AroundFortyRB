# Poll the Analytics CDC port until Ctrl+C. Output one JSON record per line.
param(
    [Parameter(Mandatory=$true)][string]$Port,
    [ValidateRange(0,2147483647)][int]$Samples = 0
)

$ErrorActionPreference = 'Stop'
$taken = 0
while ($Samples -eq 0 -or $taken -lt $Samples) {
    $serial = [IO.Ports.SerialPort]::new($Port, 115200)
    try {
        $serial.ReadTimeout = 2000
        $serial.WriteTimeout = 2000
        $serial.DtrEnable = $true
        $serial.Open()
        Start-Sleep -Milliseconds 400
        $serial.DiscardInBuffer()
        # Terminate any partial command left by an earlier session.
        $serial.WriteLine('')
        $null = $serial.ReadLine()
        while ($Samples -eq 0 -or $taken -lt $Samples) {
            $serial.WriteLine('DIAG')
            $reply = $serial.ReadLine() | ConvertFrom-Json
            if ($reply.error -eq 'unknown_command') {
                $reply = @{firmware = 'without DIAG'}
            } elseif ($null -eq $reply.usb_events_peak) {
                throw 'Unexpected DIAG response'
            }
            [ordered]@{time = [DateTimeOffset]::Now.ToString('o'); state = 'responding'; diag = $reply} |
                ConvertTo-Json -Compress
            $taken++
            Start-Sleep -Seconds 1
        }
    } catch {
        [ordered]@{time = [DateTimeOffset]::Now.ToString('o'); state = 'unavailable'; detail = $_.Exception.Message} |
            ConvertTo-Json -Compress
        $taken++
    } finally {
        $serial.Dispose()
    }
    if ($Samples -eq 0 -or $taken -lt $Samples) {
        Start-Sleep -Seconds 1
    }
}

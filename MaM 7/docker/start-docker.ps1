# Starts Docker Desktop. Moves aside socket directories left over from the previous run first,
# because on this machine Docker cannot replace its own stale AF_UNIX socket files.
$la = $env:LOCALAPPDATA
if (Get-Process | Where-Object { $_.Name -match '^com\.docker|^Docker Desktop$' }) {
    "Docker Desktop processes already running, not touching anything"
} else {
    $stamp = "stale-" + (Get-Date -Format "yyyyMMdd-HHmmss")
    foreach ($dir in "$la\Docker\run", "$la\docker-secrets-engine") {
        if (Test-Path $dir) {
            Rename-Item $dir ((Split-Path $dir -Leaf) + ".$stamp")
            "moved aside $dir"
        }
    }
    Start-Process "C:\Program Files\Docker\Docker\Docker Desktop.exe"
}

foreach ($i in 1..60) {
    docker ps *> $null
    if ($LASTEXITCODE -eq 0) { "docker ready"; exit 0 }
    Start-Sleep -Seconds 3
}
"docker not ready"
docker ps
exit 1

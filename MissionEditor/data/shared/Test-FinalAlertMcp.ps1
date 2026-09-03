[CmdletBinding()]
param(
    [Parameter()]
    [ValidateNotNullOrEmpty()]
    [string]$BaseUrl = 'http://127.0.0.1:7462',

    [Parameter()]
    [ValidatePattern('^/')]
    [string]$Endpoint = '/mcp',

    [Parameter()]
    [AllowEmptyString()]
    [string]$Token = $env:FINALALERT_MCP_TOKEN,

    [Parameter()]
    [ValidateRange(1, 300)]
    [int]$TimeoutSec = 15
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$BaseUrl = $BaseUrl.TrimEnd('/')
$healthUri = "$BaseUrl/health"
$mcpUri = "$BaseUrl$Endpoint"
$legacyVersion = '2025-11-25'
$modernVersion = '2026-07-28'
$clientInfo = @{
    name = 'finalalert-mcp-smoke-test'
    version = '1.0.0'
}
$expectedTools = @(
    'finalalert.get_status',
    'finalalert.get_map_info',
    'finalalert.get_cells',
    'finalalert.list_catalog',
    'finalalert.list_objects',
    'finalalert.get_preview',
    'finalalert.validate_map',
    'finalalert.get_ini_section',
    'finalalert.create_map',
    'finalalert.import_semantic_image',
    'finalalert.open_map',
    'finalalert.save_map',
    'finalalert.set_map_metadata',
    'finalalert.set_cells',
    'finalalert.paint_region',
    'finalalert.place_resources',
    'finalalert.place_waypoint',
    'finalalert.set_player_spawns',
    'finalalert.place_object',
    'finalalert.delete_at',
    'finalalert.set_ini_value',
    'finalalert.apply_map_plan',
    'finalalert.undo_terrain',
    'finalalert.redo_terrain'
)

function New-RequestHeaders {
    param(
        [hashtable]$Additional = @{}
    )

    $headers = @{
        Accept = 'application/json, text/event-stream'
    }
    if (-not [string]::IsNullOrWhiteSpace($Token)) {
        $headers['Authorization'] = "Bearer $Token"
    }
    foreach ($entry in $Additional.GetEnumerator()) {
        $headers[$entry.Key] = $entry.Value
    }
    return $headers
}

function Invoke-TestHttp {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Uri,

        [Parameter(Mandatory = $true)]
        [ValidateSet('GET', 'POST', 'DELETE')]
        [string]$Method,

        [hashtable]$Headers = @{},

        [AllowNull()]
        [object]$JsonBody = $null
    )

    $request = @{
        Uri = $Uri
        Method = $Method
        Headers = (New-RequestHeaders -Additional $Headers)
        TimeoutSec = $TimeoutSec
        UseBasicParsing = $true
    }
    if ($null -ne $JsonBody) {
        $json = $JsonBody | ConvertTo-Json -Depth 30 -Compress
        $request['ContentType'] = 'application/json; charset=utf-8'
        $request['Body'] = [System.Text.Encoding]::UTF8.GetBytes($json)
    }

    return Invoke-WebRequest @request
}

function ConvertFrom-TestResponse {
    param(
        [Parameter(Mandatory = $true)]
        [object]$Response
    )

    $content = [string]$Response.Content
    if ([string]::IsNullOrWhiteSpace($content)) {
        return $null
    }
    return $content | ConvertFrom-Json
}

function Get-TestResponseHeader {
    param(
        [Parameter(Mandatory = $true)]
        [object]$Response,

        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $value = $Response.Headers[$Name]
    if ($null -eq $value) {
        return $null
    }
    return (@($value) -join ',')
}

function Assert-Test {
    param(
        [Parameter(Mandatory = $true)]
        [bool]$Condition,

        [Parameter(Mandatory = $true)]
        [string]$Message
    )

    if (-not $Condition) {
        throw "FAILED: $Message"
    }
    Write-Host "[OK] $Message" -ForegroundColor Green
}

function Assert-ToolCatalog {
    param(
        [Parameter(Mandatory = $true)]
        [object]$Envelope,

        [Parameter(Mandatory = $true)]
        [string]$Mode
    )

    $names = @($Envelope.result.tools | ForEach-Object { $_.name })
    $missing = @($expectedTools | Where-Object { $names -notcontains $_ })
    Assert-Test -Condition ($missing.Count -eq 0) -Message "$Mode tools/list contains all $($expectedTools.Count) expected tools"
}

Write-Host "FinalAlert 2 MCP smoke test: $mcpUri"
Write-Host 'The token, if supplied, is intentionally not printed.'

# Health is outside JSON-RPC but proves that the editor listener is alive.
$healthResponse = Invoke-TestHttp -Uri $healthUri -Method GET
$health = ConvertFrom-TestResponse -Response $healthResponse
Assert-Test -Condition ($healthResponse.StatusCode -eq 200) -Message 'GET /health returned HTTP 200'
Assert-Test -Condition ($health.status -eq 'ok') -Message 'GET /health reports status=ok'
Assert-Test -Condition ($health.mcpEndpoint -eq '/mcp') -Message 'GET /health reports /mcp endpoint'

# Legacy 2025-11-25: initialize -> initialized -> list/call -> DELETE session.
$legacyInitializeBody = @{
    jsonrpc = '2.0'
    id = 1001
    method = 'initialize'
    params = @{
        protocolVersion = $legacyVersion
        capabilities = @{}
        clientInfo = $clientInfo
    }
}
$legacyInitializeResponse = Invoke-TestHttp -Uri $mcpUri -Method POST -JsonBody $legacyInitializeBody
$legacyInitialize = ConvertFrom-TestResponse -Response $legacyInitializeResponse
$legacySessionId = Get-TestResponseHeader -Response $legacyInitializeResponse -Name 'MCP-Session-Id'
Assert-Test -Condition ($legacyInitializeResponse.StatusCode -eq 200) -Message 'legacy initialize returned HTTP 200'
Assert-Test -Condition ($legacyInitialize.result.protocolVersion -eq $legacyVersion) -Message 'legacy initialize negotiated 2025-11-25'
Assert-Test -Condition (-not [string]::IsNullOrWhiteSpace($legacySessionId)) -Message 'legacy initialize returned MCP-Session-Id'

$legacyHeaders = @{
    'MCP-Protocol-Version' = $legacyVersion
    'MCP-Session-Id' = $legacySessionId
}
$legacyInitializedBody = @{
    jsonrpc = '2.0'
    method = 'notifications/initialized'
    params = @{}
}
$legacyInitializedResponse = Invoke-TestHttp -Uri $mcpUri -Method POST -Headers $legacyHeaders -JsonBody $legacyInitializedBody
Assert-Test -Condition ($legacyInitializedResponse.StatusCode -eq 202) -Message 'legacy notifications/initialized returned HTTP 202'

$legacyListBody = @{
    jsonrpc = '2.0'
    id = 1002
    method = 'tools/list'
    params = @{}
}
$legacyListResponse = Invoke-TestHttp -Uri $mcpUri -Method POST -Headers $legacyHeaders -JsonBody $legacyListBody
$legacyList = ConvertFrom-TestResponse -Response $legacyListResponse
Assert-Test -Condition ($legacyListResponse.StatusCode -eq 200) -Message 'legacy tools/list returned HTTP 200'
Assert-ToolCatalog -Envelope $legacyList -Mode 'legacy'

$legacyStatusBody = @{
    jsonrpc = '2.0'
    id = 1003
    method = 'tools/call'
    params = @{
        name = 'finalalert.get_status'
        arguments = @{}
    }
}
$legacyStatusResponse = Invoke-TestHttp -Uri $mcpUri -Method POST -Headers $legacyHeaders -JsonBody $legacyStatusBody
$legacyStatus = ConvertFrom-TestResponse -Response $legacyStatusResponse
Assert-Test -Condition ($legacyStatusResponse.StatusCode -eq 200) -Message 'legacy finalalert.get_status returned HTTP 200'
Assert-Test -Condition ($legacyStatus.result.isError -ne $true) -Message 'legacy finalalert.get_status succeeded'

$legacyDeleteResponse = Invoke-TestHttp -Uri $mcpUri -Method DELETE -Headers $legacyHeaders
Assert-Test -Condition ($legacyDeleteResponse.StatusCode -eq 204) -Message 'legacy DELETE closed the session with HTTP 204'

# Modern 2026-07-28: every request is self-contained and has matching headers/meta.
$modernMeta = @{
    'io.modelcontextprotocol/protocolVersion' = $modernVersion
    'io.modelcontextprotocol/clientInfo' = $clientInfo
    'io.modelcontextprotocol/clientCapabilities' = @{}
}

$modernDiscoverHeaders = @{
    'MCP-Protocol-Version' = $modernVersion
    'Mcp-Method' = 'server/discover'
}
$modernDiscoverBody = @{
    jsonrpc = '2.0'
    id = 2001
    method = 'server/discover'
    params = @{
        _meta = $modernMeta
    }
}
$modernDiscoverResponse = Invoke-TestHttp -Uri $mcpUri -Method POST -Headers $modernDiscoverHeaders -JsonBody $modernDiscoverBody
$modernDiscover = ConvertFrom-TestResponse -Response $modernDiscoverResponse
Assert-Test -Condition ($modernDiscoverResponse.StatusCode -eq 200) -Message 'modern server/discover returned HTTP 200'
Assert-Test -Condition ($modernDiscover.result.resultType -eq 'complete') -Message 'modern server/discover returned resultType=complete'
Assert-Test -Condition (@($modernDiscover.result.supportedVersions) -contains $modernVersion) -Message 'modern server/discover advertises 2026-07-28'

$modernListHeaders = @{
    'MCP-Protocol-Version' = $modernVersion
    'Mcp-Method' = 'tools/list'
}
$modernListBody = @{
    jsonrpc = '2.0'
    id = 2002
    method = 'tools/list'
    params = @{
        _meta = $modernMeta
    }
}
$modernListResponse = Invoke-TestHttp -Uri $mcpUri -Method POST -Headers $modernListHeaders -JsonBody $modernListBody
$modernList = ConvertFrom-TestResponse -Response $modernListResponse
Assert-Test -Condition ($modernListResponse.StatusCode -eq 200) -Message 'modern tools/list returned HTTP 200'
Assert-Test -Condition ($modernList.result.resultType -eq 'complete') -Message 'modern tools/list returned resultType=complete'
Assert-ToolCatalog -Envelope $modernList -Mode 'modern'

$modernStatusHeaders = @{
    'MCP-Protocol-Version' = $modernVersion
    'Mcp-Method' = 'tools/call'
    'Mcp-Name' = 'finalalert.get_status'
}
$modernStatusBody = @{
    jsonrpc = '2.0'
    id = 2003
    method = 'tools/call'
    params = @{
        name = 'finalalert.get_status'
        arguments = @{}
        _meta = $modernMeta
    }
}
$modernStatusResponse = Invoke-TestHttp -Uri $mcpUri -Method POST -Headers $modernStatusHeaders -JsonBody $modernStatusBody
$modernStatus = ConvertFrom-TestResponse -Response $modernStatusResponse
Assert-Test -Condition ($modernStatusResponse.StatusCode -eq 200) -Message 'modern finalalert.get_status returned HTTP 200'
Assert-Test -Condition ($modernStatus.result.resultType -eq 'complete') -Message 'modern finalalert.get_status returned resultType=complete'
Assert-Test -Condition ($modernStatus.result.isError -ne $true) -Message 'modern finalalert.get_status succeeded'

Write-Host 'All FinalAlert 2 MCP smoke tests passed.' -ForegroundColor Cyan

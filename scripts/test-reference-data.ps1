$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'toolchain.ps1')

$tool = Join-Path $WorkspaceRoot 'build\host\gbx_inspect.exe'
if (-not (Test-Path -LiteralPath $tool)) {
    & (Join-Path $PSScriptRoot 'build-host-tools.ps1')
}
$pakTool = Join-Path $WorkspaceRoot 'build\host\pak_inspect.exe'
if (-not (Test-Path -LiteralPath $pakTool)) {
    & (Join-Path $PSScriptRoot 'build-host-tools.ps1')
}

$referenceRoot = Join-Path $WorkspaceRoot 'TmNationsForever'
$gameDataRoot = Join-Path $referenceRoot 'GameData'

$audioTest = Join-Path $WorkspaceRoot 'build\host\audio_ring_buffer_test.exe'
$raceSounds = @(
    'RaceCheckPoint.wav',
    'RaceFinish.wav',
    'Race1.wav',
    'Race2.wav',
    'Race3.wav',
    'RaceGo.wav'
) | ForEach-Object { Join-Path $gameDataRoot "Interface\Media\Audio\Sound\Wav\$_" }
$engineSounds = @(
    'SportCarEngineIdle.wav',
    'SportCarEngineLow.wav',
    'SportCarEngineMid.wav',
    'SportCarEngineFast.wav',
    'SportCarEngineRev.wav',
    'SportCarEngineRev2.wav'
) | ForEach-Object { Join-Path $gameDataRoot "Vehicles\Media\Audio\Sound\WavData\$_" }
& $audioTest @raceSounds @engineSounds
if ($LASTEXITCODE -ne 0) {
    throw 'Original race-event or engine PCM16 WAV decode failed'
}
Write-Output 'Original race-event/engine WAV decode: 12/12 passed at 48 kHz stereo output'

$files = @(Get-ChildItem -LiteralPath $referenceRoot -Recurse -File |
    Where-Object Extension -IEq '.gbx')
$failures = [Collections.Generic.List[string]]::new()

foreach ($file in $files) {
    & $tool $file.FullName *> $null
    if ($LASTEXITCODE -ne 0) {
        $failures.Add($file.FullName)
    }
}

Write-Output "GBX reference corpus: $($files.Count - $failures.Count)/$($files.Count) passed"
if ($failures.Count -ne 0) {
    $failures
    throw "$($failures.Count) reference GBX files failed to parse"
}

$bodyTool = Join-Path $WorkspaceRoot 'build\host\gbx_body_inspect.exe'
$bodyFailures = [Collections.Generic.List[string]]::new()
$compressedBodyCount = 0
$uncompressedBodyCount = 0
foreach ($file in $files) {
    $bodyOutput = @(& $bodyTool $file.FullName 2>&1)
    if ($LASTEXITCODE -ne 0) {
        $bodyFailures.Add($file.FullName)
        continue
    }
    $body = ($bodyOutput -join "`n") | ConvertFrom-Json
    if ($body.compression -eq 'C') {
        $compressedBodyCount++
    } else {
        $uncompressedBodyCount++
    }
}
Write-Output "GBX body decode: $($files.Count - $bodyFailures.Count)/$($files.Count) passed ($compressedBodyCount LZO, $uncompressedBodyCount raw)"
if ($bodyFailures.Count -ne 0) {
    $bodyFailures
    throw "$($bodyFailures.Count) reference GBX bodies failed to decode"
}
$a01Path = Join-Path $referenceRoot 'GameData\Tracks\Campaigns\Nations\White\A01-Race.Challenge.Gbx'
$a01Body = (@(& $bodyTool $a01Path 2>&1) -join "`n") | ConvertFrom-Json
if ($a01Body.size -ne 10035 -or $a01Body.md5 -ne 'BE28D5E55D3FF26F8027712215E3EFC5' -or
    $a01Body.firstChunkId -ne '0x0304300D') {
    throw 'A01 decompressed body regression did not match'
}

$challengeTool = Join-Path $WorkspaceRoot 'build\host\challenge_inspect.exe'
$challengeFiles = @($files | Where-Object Name -Like '*.Challenge.Gbx')
$challengeFailures = [Collections.Generic.List[string]]::new()
$challengeBlockCount = 0
$skinnedBlockCount = 0
$challengeCheckpointCount = 0
$challengeLapRaceCount = 0
$nationsChallengeRoot = Join-Path $gameDataRoot 'Tracks\Campaigns\Nations'
$nationsChallengesByUid = @{}
foreach ($file in $challengeFiles) {
    $challengeOutput = @(& $challengeTool $file.FullName 2>&1)
    if ($LASTEXITCODE -ne 0) {
        $challengeFailures.Add($file.FullName)
        continue
    }
    $challenge = ($challengeOutput -join "`n") | ConvertFrom-Json
    $challengeBlockCount += $challenge.blockCount
    $skinnedBlockCount += $challenge.skinnedBlockCount
    $challengeCheckpointCount += $challenge.checkpointCount
    if ($challenge.isLapRace) {
        $challengeLapRaceCount++
    }
    if ($file.FullName.StartsWith($nationsChallengeRoot, [StringComparison]::OrdinalIgnoreCase)) {
        if ($nationsChallengesByUid.ContainsKey($challenge.mapUid)) {
            throw "Duplicate Nations challenge UID: $($challenge.mapUid)"
        }
        if ($challenge.needUnlock -or $null -eq $challenge.medalTimes.bronze -or
            $null -eq $challenge.medalTimes.silver -or $null -eq $challenge.medalTimes.gold -or
            $null -eq $challenge.medalTimes.author) {
            throw "Nations challenge selector metadata is incomplete: $($file.FullName)"
        }
        $nationsChallengesByUid[$challenge.mapUid] = $challenge.mapName
    }
}
if ($challengeFailures.Count -ne 0) {
    $challengeFailures
    throw "$($challengeFailures.Count) challenge block streams failed to parse"
}
if ($challengeFiles.Count -ne 72 -or $challengeBlockCount -ne 50576 -or
    $challengeCheckpointCount -ne 247 -or
    $challengeLapRaceCount -ne 10 -or $skinnedBlockCount -ne 6187) {
    throw 'Challenge corpus totals did not match'
}
$a01Challenge = (@(& $challengeTool $a01Path 2>&1) -join "`n") | ConvertFrom-Json
if ($a01Challenge.mapUid -ne 'BeySZdnfuSh4nHY5xztiXLmlrXe' -or
    $a01Challenge.mapName -ne 'A01-Race' -or $a01Challenge.collection -ne 'Stadium' -or
    $a01Challenge.decoration -ne 'Sunset' -or $a01Challenge.blockDataVersion -ne 1 -or
    $a01Challenge.medalTimes.bronze -ne 38030 -or
    $a01Challenge.medalTimes.silver -ne 29190 -or
    $a01Challenge.medalTimes.gold -ne 25870 -or
    $a01Challenge.medalTimes.author -ne 24540 -or
    $a01Challenge.cost -ne 1662 -or $a01Challenge.authorScore -ne 32 -or
    $a01Challenge.isLapRace -or $a01Challenge.storedNbLaps -ne 3 -or
    $a01Challenge.checkpointCount -ne 2 -or
    ($a01Challenge.checkpoints[0] -join ',') -ne '30,2,9' -or
    ($a01Challenge.checkpoints[1] -join ',') -ne '15,10,5' -or
    $a01Challenge.blockCount -ne 397 -or $a01Challenge.uniqueBlockNameCount -ne 32 -or
    $a01Challenge.blockMd5 -ne 'A84B225A04F48FBBC9308F34D1EE6B27') {
    throw 'A01 challenge placement regression did not match'
}
$a04Path = Join-Path $referenceRoot 'GameData\Tracks\Campaigns\Nations\White\A04-Acrobatic.Challenge.Gbx'
$a04Challenge = (@(& $challengeTool $a04Path 2>&1) -join "`n") | ConvertFrom-Json
if ($a04Challenge.blockCount -ne 364 -or $a04Challenge.skinnedBlockCount -ne 1 -or
    $a04Challenge.blockMd5 -ne 'E679D91F26A8CF042F13C1293323C990') {
    throw 'A04 skinned challenge placement regression did not match'
}
Write-Output "Challenge placements: $($challengeFiles.Count)/$($challengeFiles.Count) passed ($challengeBlockCount blocks, $skinnedBlockCount skinned, $challengeCheckpointCount checkpoints)"

$campaignTool = Join-Path $WorkspaceRoot 'build\host\campaign_inspect.exe'
$campaignPath = Join-Path $nationsChallengeRoot 'Nations.ConstructionCampaign.Gbx'
$campaignOutput = @(& $campaignTool $campaignPath 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Nations campaign failed to parse:`n$($campaignOutput -join "`n")"
}
$campaign = ($campaignOutput -join "`n") | ConvertFrom-Json
$expectedGroupNames = @('White', 'Green', 'Blue', 'Red', 'Black')
$expectedGroupCounts = @(15, 15, 15, 16, 5)
$expectedMapPrefixes = @('A', 'B', 'C', 'D', 'E')
$resolvedCounts = [Collections.Generic.List[int]]::new()
$unresolvedUids = [Collections.Generic.List[string]]::new()
if ($campaign.groupCount -ne 5 -or $campaign.mapCount -ne 66 -or
    $nationsChallengesByUid.Count -ne 65) {
    throw 'Nations campaign or loose challenge totals did not match'
}
for ($groupIndex = 0; $groupIndex -lt $campaign.groups.Count; $groupIndex++) {
    $group = $campaign.groups[$groupIndex]
    if ($group.name -cne $expectedGroupNames[$groupIndex] -or
        $group.maps.Count -ne $expectedGroupCounts[$groupIndex]) {
        throw "Nations campaign group $groupIndex did not match"
    }
    $resolvedOrdinal = 0
    foreach ($map in $group.maps) {
        if ($map.collection -cne 'Stadium' -or [string]::IsNullOrEmpty($map.author)) {
            throw "Nations campaign map identity did not match: $($map.uid)"
        }
        if (-not $nationsChallengesByUid.ContainsKey($map.uid)) {
            $unresolvedUids.Add($map.uid)
            continue
        }
        $resolvedOrdinal++
        $expectedNamePrefix = '{0}{1:D2}-' -f $expectedMapPrefixes[$groupIndex], $resolvedOrdinal
        if (-not $nationsChallengesByUid[$map.uid].StartsWith(
                $expectedNamePrefix, [StringComparison]::Ordinal)) {
            throw "Nations campaign order did not match at $($group.name) map $resolvedOrdinal"
        }
    }
    $resolvedCounts.Add($resolvedOrdinal)
}
if (($resolvedCounts -join ',') -cne '15,15,15,15,5' -or $unresolvedUids.Count -ne 1 -or
    $unresolvedUids[0] -cne 'Nk8jftxeWaIICIOEGV8M_cV3gba') {
    throw 'Nations campaign UID resolution did not match'
}
Write-Output 'Nations campaign order: 65/66 UIDs resolved in White/Green/Blue/Red/Black order (1 stale Red UID omitted)'

$pakFiles = @(Get-ChildItem -LiteralPath $referenceRoot -Recurse -File |
    Where-Object Extension -IEq '.pak')
$pakFailures = [Collections.Generic.List[string]]::new()

foreach ($file in $pakFiles) {
    & $pakTool $file.FullName *> $null
    if ($LASTEXITCODE -ne 0) {
        $pakFailures.Add($file.FullName)
    }
}

Write-Output "NadeoPak reference corpus: $($pakFiles.Count - $pakFailures.Count)/$($pakFiles.Count) passed"
if ($pakFailures.Count -ne 0) {
    $pakFailures
    throw "$($pakFailures.Count) reference NadeoPak files failed to parse"
}

$packListTool = Join-Path $WorkspaceRoot 'build\host\packlist_inspect.exe'
$packListPath = Join-Path $referenceRoot 'Packs\packlist.dat'
& $packListTool $packListPath *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Reference packlist.dat failed to parse'
}
Write-Output 'Pack list: 1/1 passed'

$archiveTool = Join-Path $WorkspaceRoot 'build\host\archive_inspect.exe'
$archiveFailures = [Collections.Generic.List[string]]::new()
foreach ($file in $pakFiles) {
    & $archiveTool $file.FullName $packListPath *> $null
    if ($LASTEXITCODE -ne 0) {
        $archiveFailures.Add($file.FullName)
    }
}
Write-Output "NadeoPak indexes: $($pakFiles.Count - $archiveFailures.Count)/$($pakFiles.Count) passed"
if ($archiveFailures.Count -ne 0) {
    $archiveFailures
    throw "$($archiveFailures.Count) NadeoPak indexes failed to parse"
}

$extractTool = Join-Path $WorkspaceRoot 'build\host\archive_extract.exe'
$resourcePak = Join-Path $referenceRoot 'Packs\Resource.pak'
$extractRoot = Join-Path $WorkspaceRoot 'build\reference-extract'
$ddsOutput = Join-Path $extractRoot 'NoBitmap.dds'
$gbxOutput = Join-Path $extractRoot 'WhiteFlare1.Texture.Gbx'
$meshDdsOutput = Join-Path $extractRoot 'MeshDefault.dds'
$hotGridDdsOutput = Join-Path $extractRoot 'HotGrid.dds'
$tgaOutput = Join-Path $extractRoot 'WhiteFlare1.tga'
& $extractTool $resourcePak $packListPath 'media/texture/image/nobitmap.dds' $ddsOutput *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Reference DDS archive entry failed to extract'
}
& $extractTool $resourcePak $packListPath 'Media\Texture\Image\MeshDefault.dds' $meshDdsOutput *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Reference BC1 DDS archive entry failed to extract'
}
& $extractTool $resourcePak $packListPath 'Media\Texture\Image\HotGrid.dds' $hotGridDdsOutput *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Reference BC3 DDS archive entry failed to extract'
}
& $extractTool $resourcePak $packListPath 'Media\Texture\Image\WhiteFlare1.tga' $tgaOutput *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Reference TGA archive entry failed to extract'
}
& $extractTool $resourcePak $packListPath 'Media\Texture\WhiteFlare1.Texture.gbx' $gbxOutput *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Reference GBX archive entry failed to extract'
}
$ddsBytes = [IO.File]::ReadAllBytes($ddsOutput)
if ($ddsBytes.Length -ne 824 -or [Text.Encoding]::ASCII.GetString($ddsBytes, 0, 4) -ne 'DDS ') {
    throw 'Extracted reference DDS failed its signature or size check'
}
& $tool $gbxOutput *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Extracted reference GBX failed to parse'
}

$imageTool = Join-Path $WorkspaceRoot 'build\host\image_inspect.exe'
if (-not (Test-Path -LiteralPath $imageTool)) {
    & (Join-Path $PSScriptRoot 'build-host-tools.ps1')
}
function Assert-DecodedImage {
    param(
        [Parameter(Mandatory)] [string] $Path,
        [Parameter(Mandatory)] [int] $Width,
        [Parameter(Mandatory)] [int] $Height,
        [Parameter(Mandatory)] [string] $PixelMd5
    )
    $output = @(& $imageTool $Path 2>&1)
    if ($LASTEXITCODE -ne 0) {
        throw "Reference image failed to decode: $Path`n$($output -join "`n")"
    }
    $decoded = ($output -join "`n") | ConvertFrom-Json
    if ($decoded.width -ne $Width -or $decoded.height -ne $Height -or
        $decoded.pixelMd5 -ne $PixelMd5) {
        throw "Reference image decoded incorrectly: $Path"
    }
}
Assert-DecodedImage $ddsOutput 32 32 '6E8CEF8D5FB76764FCB74E685E911EDF'
Assert-DecodedImage $meshDdsOutput 64 64 '0398BE9DF6B7B42B16AEF979FF1E4DCF'
Assert-DecodedImage $hotGridDdsOutput 64 64 '2EB1B3DEA103C1383AF233148B368836'
Assert-DecodedImage $tgaOutput 64 64 '01777DB8292DFAA8930623D5B6EE1C62'
Write-Output 'NadeoPak extraction/texture probes: 5/5 passed'
$hudFontPath = Join-Path $gameDataRoot 'Interface\Media\Font\Led_Textures\Led_00.dds'
Assert-DecodedImage $hudFontPath 512 256 '8A10740E85BB62B4A4393FCA08D85BB2'
Write-Output 'Original LED HUD font atlas: 1/1 passed'

$gbxProbeTool = Join-Path $WorkspaceRoot 'build\host\archive_gbx_probe.exe'
$stadiumPak = Join-Path $referenceRoot 'Packs\Stadium.pak'
$gbxProbeOutput = @(& $gbxProbeTool $stadiumPak $packListPath 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw 'Reference Stadium GBX stream probe failed to run'
}
$rootSeedEntries = @(
    'Stadium\ConstructionBlockInfo\ConstructionBlockInfoRoad\StadiumTubeRoad.TMEDRoad.Gbx',
    'Stadium\ConstructionBlockInfo\ConstructionBlockInfoRoad\StadiumRoadDirtHigh.TMEDRoad.Gbx',
    'Stadium\ConstructionBlockInfo\ConstructionBlockInfoRoad\StadiumControlRoadGlass.TMEDRoad.Gbx'
)
foreach ($entry in $rootSeedEntries) {
    $pattern = '^PASS\t' + [Regex]::Escape($entry) + '\t.*\trequired$'
    if (-not ($gbxProbeOutput | Where-Object { "$_" -match $pattern })) {
        throw "Reference GBX did not validate its parser-driven root cipher seed: $entry"
    }
}
Write-Output "NadeoPak parser-driven root seeds: $($rootSeedEntries.Count)/$($rootSeedEntries.Count) passed"

$recoverTool = Join-Path $WorkspaceRoot 'build\host\archive_recover.exe'
$soundOutput = Join-Path $extractRoot 'Sound.Solid.recovered.Gbx'
$soundRecovery = @(& $recoverTool $resourcePak $packListPath 'Media\Solid\Sound.Solid.Gbx' $soundOutput 2>&1)
if ($LASTEXITCODE -ne 0) {
    $soundRecovery
    throw 'Reference nested-seed GBX recovery failed'
}
if (-not ($soundRecovery | Where-Object { "$_" -eq 'cycle 2: 0x1FF0FE07F86E4317' })) {
    throw 'Reference nested-seed GBX recovery produced the wrong cipher correction'
}
$soundHash = (Get-FileHash -LiteralPath $soundOutput -Algorithm SHA256).Hash
if ($soundHash -ne 'DC686C5506F0B67F886EB1C7789197D50266CFA5CCB360194C7042D225CDC224') {
    throw 'Reference nested-seed GBX recovery produced the wrong logical file'
}
& $tool $soundOutput *> $null
if ($LASTEXITCODE -ne 0) {
    throw 'Recovered nested-seed GBX failed to parse'
}
Write-Output 'NadeoPak nested-seed recovery: 1/1 passed'

$solidTool = Join-Path $WorkspaceRoot 'build\host\archive_solid_probe.exe'
$solidOutput = @(& $solidTool $stadiumPak $packListPath 'Media\Solid\Tube\Road\BaseGround.Solid.Gbx' 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Reference packed solid failed to parse:`n$($solidOutput -join "`n")"
}
$solid = ($solidOutput -join "`n") | ConvertFrom-Json
if ($solid.storedPath -ne 'Stadium\Media\Solid\E11CFB84BECB8590C7832182A5B1718D70' -or
    $solid.rootTreeName -ne 'TubeRoad_BaseGround' -or $solid.nodeCount -ne 25 -or
    $solid.externalNodeCount -ne 4 -or $solid.visualCount -ne 7 -or
    $solid.vertexCount -ne 2697 -or $solid.indexCount -ne 4254 -or
    $solid.collisionShapeCount -ne 1 -or $solid.collisionVertexCount -ne 104 -or
    $solid.collisionIndexCount -ne 558 -or
    $solid.collisionShapes[0].positionMd5 -ne 'CC9A7BBC6B8D9136EB6D385F49FCA86B' -or
    $solid.collisionShapes[0].indexMd5 -ne 'D2AEE7F055F44CCC39F5934A70B1079A') {
    throw 'Reference packed solid graph metadata did not match'
}
$positionHashes = @(
    'E0879B9DC91B4CEF9A4E5018A7BA302A',
    'AC433B82B754AAEF52C5245A248E76ED',
    '1C6C0C11FB44A8032637DF38D5AB2250',
    '59DB5916112C9A8D8BA0455C93A4C896',
    '894C53AB06FE325CE1CF4EEDCF307429',
    '2EB5A5A5341E7E4362AE9199763EB475',
    '52605E2C30FE8871A0A1346A91822FF7'
)
$indexHashes = @(
    '83CB03B0E9DCA93E221FDF91E4EEA3E7',
    'F7EC791E7962191BB187D4D76ABD3787',
    'C53F5B53BF1E2115DC2AF0E420E2094C',
    '8FAE78AEF4FCAD4CF606B175E718983B',
    '8FAE78AEF4FCAD4CF606B175E718983B',
    '12FDBC947B1394DFF07CB9148D3CD093',
    '12FDBC947B1394DFF07CB9148D3CD093'
)
for ($index = 0; $index -lt $positionHashes.Count; $index++) {
    if ($solid.visuals[$index].positionMd5 -ne $positionHashes[$index] -or
        $solid.visuals[$index].indexMd5 -ne $indexHashes[$index]) {
        throw "Reference packed solid geometry hash mismatch at visual $index"
    }
}
Write-Output 'NadeoPak logical-path solid graph: 1/1 passed'

$treeTransformCases = @(
    @('Media\Solid\Platform\ToRoad\ToRoadMainAir.Solid.Gbx', 0.0, 32.0, 0.0, 32.0, 0.0, 32.0, 0.0, 32.0),
    @('Media\Solid\Platform\Checkpoint\Air.Solid.Gbx', -0.787712, 32.7877, 0.0, 32.0, -0.0115891, 32.0116, 0.0, 32.0),
    @('Media\Solid\Circuit\Slope\Start\BaseAir.Solid.Gbx', 0.0, 32.0, 0.0, 32.0, 0.0, 32.0, 0.0, 32.0),
    @('Media\Solid\Circuit\Slope\Straight\BaseAir.Solid.Gbx', 0.0, 32.0, 0.0, 32.0, 0.0, 32.0, 0.0, 32.0),
    @('Media\Solid\Circuit\Slope\End\BaseAir.Solid.Gbx', 0.0, 32.0, 0.0, 32.0, 0.0, 32.0, 0.0, 32.0)
)
foreach ($case in $treeTransformCases) {
    $output = @(& $solidTool $stadiumPak $packListPath $case[0] 2>&1)
    if ($LASTEXITCODE -ne 0) {
        throw "A01 transformed solid failed to parse: $($case[0])`n$($output -join "`n")"
    }
    $transformed = ($output -join "`n") | ConvertFrom-Json
    $actual = @(
        $transformed.bounds.minimum[0], $transformed.bounds.maximum[0],
        $transformed.bounds.minimum[2], $transformed.bounds.maximum[2],
        $transformed.collisionBounds.minimum[0], $transformed.collisionBounds.maximum[0],
        $transformed.collisionBounds.minimum[2], $transformed.collisionBounds.maximum[2]
    )
    for ($index = 0; $index -lt $actual.Count; $index++) {
        if ([Math]::Abs($actual[$index] - $case[$index + 1]) -gt 0.001) {
            throw "A01 ancestor tree transform mismatch for $($case[0]) at bound $index"
        }
    }
}
Write-Output 'A01 ancestor tree transforms: 5/5 visual and collision bounds normalized'

$materialTool = Join-Path $WorkspaceRoot 'build\host\archive_material_probe.exe'
$materialOutput = @(& $materialTool $stadiumPak $packListPath 'Media\Solid\Tube\Road\BaseGround.Solid.Gbx' $gameDataRoot 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Reference packed material chain failed to resolve:`n$($materialOutput -join "`n")"
}
$materials = ($materialOutput -join "`n") | ConvertFrom-Json
if ($materials.textureCount -ne 4 -or
    ($materials.visualTextureIndices -join ',') -ne '0,1,2,3,3,3,3') {
    throw 'Reference packed material bindings did not match'
}
$expectedMaterialImages = @(
    @('Stadium\Media\Texture\Image\StadiumRoadDetailsD.dds', 1024, 1024, '72A631BB80DB69C0F474F9C311E4643B'),
    @('Stadium\Media\Texture\Image\StadiumPillarD.dds', 128, 512, 'C5B1C9625CCEC38930B45086C662B936'),
    @('Stadium\Media\Texture\Image\StadiumGrass1.dds', 1024, 1024, '21078AA473DD44A552F296323EAFB6E2'),
    @('Stadium\Media\Texture\Image\StadiumGrassFenceD.dds', 2048, 256, '2EE904039CBDB5FD797BC0D91EB798E7')
)
for ($index = 0; $index -lt $expectedMaterialImages.Count; $index++) {
    $expected = $expectedMaterialImages[$index]
    $actual = $materials.textures[$index]
    if ($actual.image -ne $expected[0] -or $actual.width -ne $expected[1] -or
        $actual.height -ne $expected[2] -or $actual.pixelMd5 -ne $expected[3]) {
        throw "Reference packed material texture mismatch at binding $index"
    }
}
Write-Output 'Packed material/bitmap/loose-image chains: 4/4 passed'

$packedImageOutput = @(& $materialTool $stadiumPak $packListPath 'Media\Solid\Zone\Water\Base.Solid.Gbx' $gameDataRoot 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Packed material image fallback failed to resolve:`n$($packedImageOutput -join "`n")"
}
$packedImageMaterials = ($packedImageOutput -join "`n") | ConvertFrom-Json
$packedImage = @($packedImageMaterials.textures | Where-Object image -CEQ 'Techno\Media\Texture\Image\SeaHeightPC2.tga')
if ($packedImage.Count -ne 1 -or $packedImage[0].width -ne 512 -or
    $packedImage[0].height -ne 512 -or
    $packedImage[0].pixelMd5 -ne '6F6126A99D3A641D70B0F34AA06AE3C8') {
    throw 'Packed material image fallback did not match'
}
Write-Output 'Packed material raw-image fallback: 1/1 passed'

$lodOutput = @(& $solidTool $stadiumPak $packListPath 'Media\Solid\Circuit\Border\Corner\InAir.Solid.Gbx' 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Reference visual-MIP solid failed to parse:`n$($lodOutput -join "`n")"
}
$lodSolid = ($lodOutput -join "`n") | ConvertFrom-Json
$enabledLodVisuals = @($lodSolid.visuals | Where-Object renderEnabled).Count
if ($lodSolid.visualCount -ne 21 -or $enabledLodVisuals -ne 11) {
    throw 'Reference visual-MIP selection did not match'
}
Write-Output 'Packed visual-MIP selection: 11/21 high-detail visuals enabled'

$vehicleOutput = @(& $solidTool $stadiumPak $packListPath 'Vehicles\Media\Solid\StadiumCar.Solid.Gbx' 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "StadiumCar solid failed to parse:`n$($vehicleOutput -join "`n")"
}
$vehicleSolid = ($vehicleOutput -join "`n") | ConvertFrom-Json
$vehicleLowLod = @($vehicleSolid.visuals | Where-Object treeName -CLike '3*')
if ($vehicleLowLod.Count -ne 7 -or
    ($vehicleLowLod | Measure-Object vertices -Sum).Sum -ne 3506 -or
    ($vehicleLowLod | Measure-Object indices -Sum).Sum -ne 10875) {
    throw 'StadiumCar low-LOD selection did not match'
}
Write-Output 'Packed StadiumCar low-LOD selection: 7/87 visuals enabled for vehicle rendering'
$wheelApplicationPoints = @{
    FLSurf = @(0.863012, 0.3525, 1.78209)
    FRSurf = @(-0.86299, 0.3525, 1.78209)
    RRSurf = @(-0.885, 0.352504, -1.2055)
    RLSurf = @(0.885002, 0.352504, -1.2055)
}
foreach ($entry in $wheelApplicationPoints.GetEnumerator()) {
    $node = @($vehicleSolid.treeTransforms | Where-Object name -CEQ $entry.Key)
    if ($node.Count -ne 1) {
        throw "StadiumCar wheel application node $($entry.Key) did not resolve exactly once"
    }
    for ($component = 0; $component -lt 9; ++$component) {
        $expected = if ($component -in 0, 4, 8) { 1.0 } else { 0.0 }
        if ([Math]::Abs($node[0].transform[$component] - $expected) -gt 0.00001) {
            throw "StadiumCar wheel application node $($entry.Key) rotation did not match"
        }
    }
    for ($component = 0; $component -lt 3; ++$component) {
        if ([Math]::Abs($node[0].transform[$component + 9] - $entry.Value[$component]) -gt 0.00001) {
            throw "StadiumCar wheel application node $($entry.Key) point did not match"
        }
    }
    $shape = @($vehicleSolid.collisionShapes | Where-Object treeName -CEQ $entry.Key)
    if ($shape.Count -ne 1 -or $shape[0].type -CNE 'ellipsoid' -or
        [Math]::Abs($shape[0].minimum[0] + 0.182) -gt 0.00001 -or
        [Math]::Abs($shape[0].minimum[1] + 0.364) -gt 0.00001 -or
        [Math]::Abs($shape[0].minimum[2] + 0.364) -gt 0.00001 -or
        [Math]::Abs($shape[0].maximum[0] - 0.182) -gt 0.00001 -or
        [Math]::Abs($shape[0].maximum[1] - 0.364) -gt 0.00001 -or
        [Math]::Abs($shape[0].maximum[2] - 0.364) -gt 0.00001) {
        throw "StadiumCar wheel collision shape $($entry.Key) did not match"
    }
}
Write-Output 'StadiumCar wheels: 4/4 composed Surf force points and ellipsoids matched'

$blockAssetsTool = Join-Path $WorkspaceRoot 'build\host\archive_block_assets.exe'
$blockAssetsOutput = @(& $blockAssetsTool $stadiumPak $packListPath $a01Path 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "A01 block asset graph failed to resolve:`n$($blockAssetsOutput -join "`n")"
}
$blockAssets = ($blockAssetsOutput -join "`n") | ConvertFrom-Json
$unresolvedBlockAssets = @($blockAssets.blocks | Where-Object {
    $_.error -or @($_.unresolved).Count -ne 0
})
if ($blockAssets.uniqueBlockNameCount -ne 32 -or
    $blockAssets.resolvedBlockInfoCount -ne 32 -or
    $blockAssets.blockNamesWithSolids -ne 31 -or
    $blockAssets.solidReferenceCount -ne 81 -or
    $blockAssets.blockNamesWithTriggers -ne 3 -or
    $blockAssets.triggerSolidReferenceCount -ne 3 -or
    $blockAssets.parsedMetadataCount -ne 4 -or
    $unresolvedBlockAssets.Count -ne 0) {
    throw 'A01 block asset graph regression did not match'
}
$waypointMetadata = @($blockAssets.blocks | Where-Object { $_.metadata })
$startMetadata = @($waypointMetadata | Where-Object name -CEQ 'StadiumRoadMainStartLine')
$finishMetadata = @($waypointMetadata | Where-Object name -CEQ 'StadiumRoadMainFinishLine')
$checkpointMetadata = @($waypointMetadata | Where-Object { $_.metadata.waypoint -eq 'checkpoint' })
$ringCheckpointMetadata = @($checkpointMetadata | Where-Object name -CEQ 'StadiumCheckpointRingV')
$platformCheckpointMetadata = @($checkpointMetadata | Where-Object name -CEQ 'StadiumPlatformCheckpoint')
$gtCurveFootprint = @($blockAssets.blocks | Where-Object name -CEQ 'StadiumCircuitBorderGTCurve2In')
$toRoadFootprint = @($blockAssets.blocks | Where-Object name -CEQ 'StadiumPlatformToRoadMain')
if ($startMetadata.Count -ne 1 -or $startMetadata[0].metadata.waypoint -ne 'start' -or
    [Math]::Abs($startMetadata[0].metadata.spawnAir[9] - 16.0) -gt 0.00001 -or
    [Math]::Abs($startMetadata[0].metadata.spawnAir[10] - 2.21) -gt 0.00001 -or
    [Math]::Abs($startMetadata[0].metadata.spawnAir[11] - 11.2) -gt 0.00001 -or
    $finishMetadata.Count -ne 1 -or $finishMetadata[0].metadata.waypoint -ne 'finish' -or
    $checkpointMetadata.Count -ne 2 -or
    $ringCheckpointMetadata.Count -ne 1 -or $ringCheckpointMetadata[0].metadata.noRespawn -ne $true -or
    $platformCheckpointMetadata.Count -ne 1 -or $platformCheckpointMetadata[0].metadata.noRespawn -ne $false -or
    $gtCurveFootprint.Count -ne 1 -or ($gtCurveFootprint[0].airFootprint -join ',') -ne '2,2' -or
    $toRoadFootprint.Count -ne 1 -or ($toRoadFootprint[0].airFootprint -join ',') -ne '1,1') {
    throw 'A01 BlockInfo waypoint/spawn metadata did not match'
}
Write-Output 'A01 construction asset graphs: 32/32 resolved (81 visible solid references, 4 waypoint metadata records)'

$a09Path = Join-Path $gameDataRoot 'Tracks\Campaigns\Nations\White\A09-Race.Challenge.Gbx'
$fabricAliasOutput = @(& $blockAssetsTool $stadiumPak $packListPath $a09Path 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "A09 fabric block alias failed to resolve:`n$($fabricAliasOutput -join "`n")"
}
$fabricAliasGraph = (($fabricAliasOutput -join "`n") | ConvertFrom-Json).blocks |
    Where-Object name -CEQ 'StadiumFabricPillarCornerOut'
if ($null -eq $fabricAliasGraph -or
    $fabricAliasGraph.blockInfo -CNE 'Stadium\ConstructionBlockInfo\ConstructionBlockInfoClassic\StadiumFabricStraight1x1Ramp.TMEDClassic.Gbx' -or
    @($fabricAliasGraph.solids | Where-Object path -CLike '*FabricPillarCornerOutGround.Solid.Gbx').Count -ne 1 -or
    @($fabricAliasGraph.solids | Where-Object path -CLike '*FabricPillarCornerOutAir.Solid.Gbx').Count -ne 1) {
    throw 'StadiumFabricPillarCornerOut packed identity alias did not match'
}
Write-Output 'Packed fabric identity alias: StadiumFabricPillarCornerOut resolved'

$trackSceneTool = Join-Path $WorkspaceRoot 'build\host\archive_track_scene.exe'
$a01GroundQueries = @(
    194.09119, 89.358284, 689.6018,
    220.84866, 88.02442, 696.987,
    975.1122, 24.013105, 310.16437,
    971.8925, 24.024632, 298.13672,
    958.2894, 24.06828, 263.10492,
    952.08325, 24.056562, 252.10118,
    945.00714, 24.04981, 241.58334,
    937.0851, 24.054468, 231.6317,
    620.3471, 26.786789, 173.39099,
    559.74493, 56.200424, 174.508,
    536.339, 67.86969, 174.91403,
    524.7381, 73.672455, 175.08028
)
$a01SceneQueries = @($a01GroundQueries) + @(
    '--trigger',
    27.559633, 113.9084, 181.13004,
    17.136797, 114.1994, 180.86806
)
$trackSceneOutput = @(& $trackSceneTool $stadiumPak $packListPath $a01Path $gameDataRoot @a01SceneQueries 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "A01 portable track scene failed to build:`n$($trackSceneOutput -join "`n")"
}
$trackScene = ($trackSceneOutput -join "`n") | ConvertFrom-Json
$maxSideFriction = @($trackScene.vehicleTuning.maxSideFriction)
$wheelCompression = @($trackScene.vehicleTuning.modulationFromWheelCompression)
$steerDriveTorque = @($trackScene.vehicleTuning.steerDriveTorque)
$steerSlowDown = @($trackScene.vehicleTuning.steerSlowDown)
$visualSteerAngle = @($trackScene.vehicleTuning.visualSteerAngleFromSpeed)
$airControlZ = @($trackScene.vehicleTuning.airControlZCoefFromAngularSpeed)
$collisionSurfaceCounts = @($trackScene.collisionSurfaceTriangleCounts)
$m6RolloverLateral = @($trackScene.vehicleTuning.m6RolloverLateralFromSpeedRatio)
$m6BurnoutRollover = @($trackScene.vehicleTuning.m6BurnoutRolloverFromSpeed)
$m6DonutRollover = @($trackScene.vehicleTuning.m6DonutRolloverFromSpeed)
$expectedBodyShapes = @{
    BodySurf = @(0.439118, 0.362, 1.90153, 1, 0, 0, 0, 0.998223, -0.059586, 0, 0.059586, 0.998223, 0, 0.471253, 0.219106)
    BodySurf01 = @(0.968297, 0.362741, 1.68228, 1, 0, 0, 0, 0.998954, -0.045716, 0, 0.045716, 0.998954, 0, 0.448782, -0.20792)
    BodySurf02 = @(1.02092, 0.515218, 1.03801, 1, 0, 0, 0, 0.9989, -0.046888, 0, 0.046888, 0.9989, 0, 0.652812, -0.89763)
    BodySurf04 = @(0.384841, 0.905323, 0.283418, 0, -1, 0, 0, 0, -1, 1, 0, 0, -0.015532, 0.363252, 1.75357)
}
$bodyShapes = @($trackScene.vehicleCollisionShapes | Where-Object name -CLike 'BodySurf*')
if (@($trackScene.vehicleCollisionShapes).Count -ne 8 -or $bodyShapes.Count -ne 4) {
    throw 'StadiumCar collision ellipsoid count did not match'
}
foreach ($entry in $expectedBodyShapes.GetEnumerator()) {
    $shape = @($bodyShapes | Where-Object name -CEQ $entry.Key)
    if ($shape.Count -ne 1 -or $shape[0].surfaceId -ne 0) {
        throw "StadiumCar $($entry.Key) identity/surface did not match"
    }
    $actual = @($shape[0].halfAxes) + @($shape[0].transform)
    if ($actual.Count -ne $entry.Value.Count) {
        throw "StadiumCar $($entry.Key) collision field count did not match"
    }
    for ($index = 0; $index -lt $actual.Count; ++$index) {
        if ([Math]::Abs($actual[$index] - $entry.Value[$index]) -gt 0.00001) {
            throw "StadiumCar $($entry.Key) collision field $index did not match"
        }
    }
}
if ($trackScene.challengeBlockCount -ne 397 -or $trackScene.placementCount -ne 373 -or
    $trackScene.invisibleBlockCount -ne 24 -or $trackScene.modelCount -ne 33 -or
    $trackScene.textureCount -ne 33 -or $trackScene.uniqueEnabledVisualCount -ne 262 -or
    $trackScene.uniqueVertexCount -ne 64516 -or $trackScene.uniqueIndexCount -ne 161715 -or
    $trackScene.instancedVertexCount -ne 264735 -or
    $trackScene.instancedIndexCount -ne 635640 -or
    $trackScene.collisionVertexCount -ne 98189 -or
    $trackScene.collisionIndexCount -ne 528984 -or
    $collisionSurfaceCounts.Count -ne 31 -or
    ($collisionSurfaceCounts -join ',') -CNE '11796,0,528,684,65862,0,0,0,0,71232,276,0,0,0,0,0,25906,0,0,0,0,0,44,0,0,0,0,0,0,0,0' -or
    $trackScene.triggerCount -ne 3 -or $trackScene.triggerVertexCount -ne 72 -or
    $trackScene.triggerIndexCount -ne 396 -or
    $trackScene.orderedCheckpointCount -ne 2 -or
    $trackScene.vehicleTuning.count -ne 30 -or
    $trackScene.vehicleTuning.activeIndex -ne 29 -or
    $trackScene.vehicleTuning.name -CNE '20juin2006' -or
    -not $trackScene.vehicleTuning.fakeEngine -or
    [Math]::Abs($trackScene.vehicleTuning.mass - 1.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.maxSpeed - 277.7778) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.reverseMaxSpeed - 138.61111) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.turboBoost - 5.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.turbo2Boost - 20.0) -gt 0.001 -or
    $trackScene.vehicleTuning.turboDuration -ne 250 -or
    $trackScene.vehicleTuning.turbo2Duration -ne 100 -or
    [Math]::Abs($trackScene.vehicleTuning.steerSpeed - 20.0) -gt 0.001 -or
    $trackScene.vehicleTuning.steerModel -ne 5 -or
    [Math]::Abs($trackScene.vehicleTuning.steerGroundTorque - 0.08) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.suspensionSpring - 40.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.suspensionDamping - 1.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.suspensionMinimum) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.suspensionMaximum - 0.7) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.suspensionRest - 0.2) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.relativeSpeedMultiplier - 0.2) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.absorbTension - 5.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.centerOfMassAftForce) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.centerOfMassDownUp - 0.45) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.inertiaMass - 5.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.inertiaHalfDiagonal[0] - 0.5) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.inertiaHalfDiagonal[1] - 0.5) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.inertiaHalfDiagonal[2] - 1.0) -gt 0.001 -or
    $trackScene.vehicleTuning.tireMaterial -ne 9 -or
    $trackScene.vehicleTuning.shockModel -ne 2 -or
    [Math]::Abs($trackScene.vehicleTuning.gravityCoef - 3.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.gravityCoefAir - 2.5) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.angularFluidFrictionCoef1 - 0.4) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.angularFluidFrictionCoef2 - 0.2) -gt 0.001 -or
    $trackScene.vehicleTuning.airControlDuration -ne 30000 -or
    [Math]::Abs($trackScene.vehicleTuning.maxAngularSpeedYAirControl - 3.14) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.bodyFrictionCoefConcrete - 0.3) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.bodyFrictionCoefMetal - 0.2) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.bodyRestCoefMetal + 0.5) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.bodyRestCoefConcrete + 0.5) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.wheelFrictionCoefConcrete) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.wheelRestCoefConcrete + 0.8) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.wheelFrictionCoefMetal) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.wheelRestCoefMetal + 0.8) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.angularSpeedYImpulseScale - 0.55) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.angularSpeedClamp - 100.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.linearSpeed2PositiveDeltaMax - 10000.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.steerLowSpeed - 30.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.lateralSlopeAdherenceMin - 0.3) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.lateralSlopeAdherenceMax - 0.7) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.axialSlopeAdherenceMin - 0.4) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.axialSlopeAdherenceMax - 0.7) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.steerGroundTorqueSlippingCoef - 1.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.sideFriction1 - 40.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.maxSideFrictionBlendCoef - 0.018) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.maxSideFrictionSliding - 0.35) -gt 0.001 -or
    $maxSideFriction.Count -ne 1 -or @($maxSideFriction[0].x).Count -ne 6 -or
    ($maxSideFriction[0].x -join ',') -CNE '0,100,200,300,400,500' -or
    ($maxSideFriction[0].y -join ',') -CNE '80,80,75,67,60,55' -or
    $steerDriveTorque.Count -ne 1 -or @($steerDriveTorque[0].x).Count -ne 10 -or
    ($steerDriveTorque[0].x -join ',') -CNE '-100,0,50,100,150,200,300,400,500,600' -or
    ($steerDriveTorque[0].y -join ',') -CNE '7,16,10.4,8.5,6.8,5.5,4.25,4,3.75,3.75' -or
    $steerSlowDown.Count -ne 1 -or @($steerSlowDown[0].x).Count -ne 6 -or
    ($steerSlowDown[0].x -join ',') -CNE '-50,0,50,100,200,300' -or
    ($steerSlowDown[0].y -join ',') -CNE '1,0,0,0,0,0' -or
    [Math]::Abs($trackScene.vehicleTuning.steerSlowDownCoef - 0.5) -gt 0.001 -or
    $visualSteerAngle.Count -ne 1 -or @($visualSteerAngle[0].x).Count -ne 2 -or
    ($visualSteerAngle[0].x -join ',') -CNE '0,100' -or
    ($visualSteerAngle[0].y -join ',') -CNE '30,10' -or
    $airControlZ.Count -ne 1 -or ($airControlZ[0].x -join ',') -CNE '0' -or
    ($airControlZ[0].y -join ',') -CNE '1' -or
    $wheelCompression.Count -ne 1 -or @($wheelCompression[0].x).Count -ne 4 -or
    ($wheelCompression[0].x -join ',') -CNE '0,0.6,0.975,1' -or
    ($wheelCompression[0].y -join ',') -CNE '0.1,0.3,1,1' -or
    [Math]::Abs($trackScene.vehicleTuning.frictionModulationWhenSlipAndBrake - 0.2) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.m6InertialMass - 5.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.m6ForceEpsilon - 0.01) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.m6InertialTorqueModulationX - 0.05) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.m6InertialTorqueModulationZ - 0.05) -gt 0.001 -or
    $m6RolloverLateral.Count -ne 1 -or @($m6RolloverLateral[0].x).Count -ne 5 -or
    ($m6RolloverLateral[0].x -join ',') -CNE '0,1,10,20,30' -or
    ($m6RolloverLateral[0].y -join ',') -CNE '0,0.8,1,1,1' -or
    $m6BurnoutRollover.Count -ne 1 -or @($m6BurnoutRollover[0].x).Count -ne 3 -or
    ($m6BurnoutRollover[0].x -join ',') -CNE '0,10,50' -or
    ($m6BurnoutRollover[0].y -join ',') -CNE '1.5,1.2,0.2' -or
    $m6DonutRollover.Count -ne 1 -or @($m6DonutRollover[0].x).Count -ne 3 -or
    ($m6DonutRollover[0].x -join ',') -CNE '0,10,20' -or
    ($m6DonutRollover[0].y -join ',') -CNE '0,5,6' -or
    [Math]::Abs($trackScene.vehicleTuning.m6MaxRpm - 11000.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.burnoutRpmAcceleration - 6000.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.airRpmAcceleration - 6000.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.airRpmDeadening - 3000.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.rpmLossOnGearUp - 17000.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.rpmGainOnGearDown - 11500.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.speedLimitPositiveForTakeOffFront - 3.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.speedLimitPositiveForTakeOffRear - 2.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.speedLimitNegativeForTakeOffFront + 2.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.speedLimitNegativeForTakeOffRear + 3.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.rpmGainOnTakeOff - 10000.0) -gt 0.001 -or
    [Math]::Abs($trackScene.vehicleTuning.rpmLossOnTakeOffFinished - 4000.0) -gt 0.001 -or
    $trackScene.vehicleCheckpointCount -ne 2 -or
    $trackScene.occupiedCollisionCellCount -ne 359 -or
    [Math]::Abs($trackScene.vehicleSpawn.x - 171.2) -gt 0.0001 -or
    [Math]::Abs($trackScene.vehicleSpawn.y - 90.21) -gt 0.0001 -or
    [Math]::Abs($trackScene.vehicleSpawn.z - 688.0) -gt 0.0001 -or
    [Math]::Abs($trackScene.vehicleSpawn.heading - 1.5708) -gt 0.0001 -or
    $trackScene.startGround.surfaceId -ne 16 -or
    $trackScene.startGroundSamples.positiveX -ne 16 -or
    $trackScene.groundQueries.count -ne 12 -or $trackScene.groundQueries.hits -ne 12 -or
    $null -ne $trackScene.groundQueries.firstMismatch -or
    $trackScene.triggerQueries.count -ne 1 -or $trackScene.triggerQueries.hits -ne 1) {
    throw 'A01 portable track scene regression did not match'
}
$soundFields = @(
    'minVolume', 'maxVolume', 'fadeInStartRpm', 'fadeInEndRpm', 'fadeOutStartRpm',
    'fadeOutEndRpm', 'minPitch', 'maxPitch', 'pitchShiftStartRpm', 'pitchShiftEndRpm'
)
$expectedSoundComponents = @(
    @('WavData\SportCarEngineIdle.wav', 0, .81, 0, 0, 600, 4000, .5, 1.4, 600, 3000),
    @('WavData\SportCarEngineLow.wav', 0, .5, 335, 600, 5000, 8000, .3, 1.2, 500, 6000),
    @('WavData\SportCarEngineMid.wav', 0, .65, 3500, 6000, 7000, 13000, .9, 1.7, 4000, 9000),
    @('WavData\SportCarEngineFast.wav', 0, .81, 5000, 9000, 9000, 14000, 1.1, 1.8, 3000, 11000),
    @('WavData\SportCarEngineRev.wav', 0, .81, -335, -600, -8000, -17000, .3, .6, -500, -12000),
    @('WavData\SportCarEngineRev2.wav', 0, .5, -4000, -6000, -9000, -14000, 1.5, 2.6, -4000, -14000)
)
if ([Math]::Abs($trackScene.soundEngine.volume - .55) -gt 0.0001 -or
    [Math]::Abs($trackScene.soundEngine.maxRpm - 11000.0) -gt 0.001 -or
    @($trackScene.soundEngine.components).Count -ne $expectedSoundComponents.Count) {
    throw 'Stadium sound-engine root did not match'
}
for ($componentIndex = 0; $componentIndex -lt $expectedSoundComponents.Count; ++$componentIndex) {
    $actual = $trackScene.soundEngine.components[$componentIndex]
    $expected = $expectedSoundComponents[$componentIndex]
    if ($actual.path -CNE $expected[0]) {
        throw "Stadium sound-engine component $componentIndex path did not match"
    }
    for ($fieldIndex = 0; $fieldIndex -lt $soundFields.Count; ++$fieldIndex) {
        if ([Math]::Abs($actual.($soundFields[$fieldIndex]) - $expected[$fieldIndex + 1]) -gt 0.0001) {
            throw "Stadium sound-engine component $componentIndex $($soundFields[$fieldIndex]) did not match"
        }
    }
}
$expectedSoundCurves = @(
    @(@(0, 100, 250), @(.7, .7, 1)),
    @(@(0, 50, 250), @(1, 1, 1)),
    @(@(0, 9000, 11000), @(1, 1, .8)),
    @(@(0, .1, 1), @(.7, .7, 1)),
    @(@(0, 100, 500), @(0, 0, 0)),
    @(@(0, 50, 250), @(0, 0, 0)),
    @(@(900, 7000, 30000), @(0, 0, 0)),
    @(@(0, 1, 10), @(0, 0, 0))
)
$actualSoundCurves = @($trackScene.soundEngine.volumeCurves) + @($trackScene.soundEngine.alphaCurves)
for ($curveIndex = 0; $curveIndex -lt $expectedSoundCurves.Count; ++$curveIndex) {
    foreach ($axis in @('x', 'y')) {
        $axisIndex = if ($axis -eq 'x') { 0 } else { 1 }
        $actual = @($actualSoundCurves[$curveIndex].$axis)
        $expected = @($expectedSoundCurves[$curveIndex][$axisIndex])
        if ($actual.Count -ne $expected.Count) {
            throw "Stadium sound-engine curve $curveIndex $axis length did not match"
        }
        for ($valueIndex = 0; $valueIndex -lt $actual.Count; ++$valueIndex) {
            if ([Math]::Abs($actual[$valueIndex] - $expected[$valueIndex]) -gt 0.0001) {
                throw "Stadium sound-engine curve $curveIndex $axis[$valueIndex] did not match"
            }
        }
    }
}
Write-Output 'Stadium engine graph: 6/6 signed-RPM components and 8/8 root curves matched'
$expectedTuningArrays = @{
    gearRatio = @(400.0, 370.0, 220.0, 160.0, 110.0, 85.0)
    maxRpmByGear = @(0.0, 0.95, 0.9, 0.95, 0.95, 1.0)
    minRpmByGear = @(0.0, 0.0, 0.5, 0.57, 0.55, 0.6)
    rpmWantedOnGearUp = @(0.0, 0.0, 0.628, 0.7, 0.7, 0.72)
}
foreach ($entry in $expectedTuningArrays.GetEnumerator()) {
    $actual = @($trackScene.vehicleTuning.($entry.Key))
    if ($actual.Count -ne $entry.Value.Count) {
        throw "Active Stadium vehicle tuning $($entry.Key) length did not match"
    }
    for ($index = 0; $index -lt $actual.Count; ++$index) {
        if ([Math]::Abs($actual[$index] - $entry.Value[$index]) -gt 0.0001) {
            throw "Active Stadium vehicle tuning $($entry.Key)[$index] did not match"
        }
    }
}
Write-Output 'Active Stadium vehicle tuning: 30 entries, suspension/inertia/material/drivetrain matched'
$expectedTopologyPlacements = @{
    'Stadium\Media\Solid\Control\RoadGlass\DeadendAir.Solid.Gbx' = 14
    'Stadium\Media\Solid\Control\RoadGlass\CornerAir.Solid.Gbx' = 4
    'Stadium\Media\Solid\Control\RoadGlass\StraightAir.Solid.Gbx' = 9
    'Stadium\Media\Solid\Tube\Road\StraightAir.Solid.Gbx' = 44
    'Stadium\Media\Solid\Tube\RoadCross\CrossAir.Solid.Gbx' = 2
}
foreach ($entry in $expectedTopologyPlacements.GetEnumerator()) {
    $actual = @($trackScene.models | Where-Object path -CEQ $entry.Key)
    if ($actual.Count -ne 1 -or $actual[0].placements -ne $entry.Value) {
        throw "A01 topology variant mismatch for $($entry.Key)"
    }
}
Write-Output 'A01 portable scene: 373 placements, 33 models, 33 textures, 635640 render indices, 528984 collision indices'

$c11Path = Join-Path $gameDataRoot 'Tracks\Campaigns\Nations\Blue\C11-Race.Challenge.Gbx'
$c11SceneOutput = @(& $trackSceneTool $stadiumPak $packListPath $c11Path $gameDataRoot 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "C11 portable track scene failed to build:`n$($c11SceneOutput -join "`n")"
}
$c11Scene = ($c11SceneOutput -join "`n") | ConvertFrom-Json
if ($c11Scene.challengeBlockCount -ne 1079 -or $c11Scene.placementCount -ne 1079 -or
    $c11Scene.modelCount -ne 61 -or $c11Scene.textureCount -ne 51 -or
    $c11Scene.triggerCount -ne 4 -or $c11Scene.vehicleCheckpointCount -ne 3) {
    throw 'C11 animated-sign scene regression did not match'
}
Write-Output 'C11 portable scene: Bink-backed animated sign left unbound without blocking geometry'

param(
    [string]$Serial = 'emulator-5554'
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'toolchain.ps1')

$adb = Join-Path $env:ANDROID_HOME 'platform-tools\adb.exe'
$sourceRoot = Join-Path $WorkspaceRoot 'TmNationsForever\Packs'
$importRoot = '/sdcard/Download/TmNationsForever'
$destination = "$importRoot/Packs"

& $adb -s $Serial shell mkdir -p $destination
if ($LASTEXITCODE -ne 0) {
    throw "Unable to create Android content directory on $Serial"
}
foreach ($name in @('packlist.dat', 'Resource.pak', 'Stadium.pak')) {
    & $adb -s $Serial push (Join-Path $sourceRoot $name) "$destination/$name"
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to deploy $name to $Serial"
    }
}
$gameDataSourceRoot = Join-Path $WorkspaceRoot 'TmNationsForever\GameData'
$gameDataDestination = "$importRoot/GameData"
foreach ($relativePath in @(
    'Interface\Advertising\Ad8x1A.dds',
    'Interface\Advertising\Ad8x1B.dds',
    'Interface\Advertising\RaceAd8x1A.dds',
    'Interface\Advertising\RaceAd8x1B.dds',
    'Interface\Advertising\RaceAd8x1C.dds',
    'Interface\Advertising\RaceAd8x1D.dds',
    'Interface\Advertising\StaticAd2x1A.dds',
    'Interface\Advertising\StaticRaceAd8x1A.dds',
    'Interface\Media\Texture\Image\Advertisement.dds',
    'Stadium\Media\Texture\Image\StadiumAirshipD.dds',
    'Stadium\Media\Texture\Image\StadiumCircuitD.dds',
    'Stadium\Media\Texture\Image\StadiumCircuitLogoD.dds',
    'Stadium\Media\Texture\Image\StadiumCircuitScreen.dds',
    'Stadium\Media\Texture\Image\StadiumControlGlassD.dds',
    'Stadium\Media\Texture\Image\StadiumControlInteriorD.dds',
    'Stadium\Media\Texture\Image\StadiumDirt1.dds',
    'Stadium\Media\Texture\Image\StadiumDirtGrid.dds',
    'Stadium\Media\Texture\Image\StadiumDirtRoad.dds',
    'Stadium\Media\Texture\Image\StadiumDirtRoadBorderD.dds',
    'Stadium\Media\Texture\Image\StadiumFabricD.dds',
    'Stadium\Media\Texture\Image\StadiumFabricFloorD.dds',
    'Stadium\Media\Texture\Image\StadiumFabricPoolD.dds',
    'Stadium\Media\Texture\Image\StadiumFabricStructureD.dds',
    'Stadium\Media\Texture\Image\StadiumFan.dds',
    'Stadium\Media\Texture\Image\StadiumGrass1.dds',
    'Stadium\Media\Texture\Image\StadiumGrassFenceD.dds',
    'Stadium\Media\Texture\Image\StadiumInflatable2D.dds',
    'Stadium\Media\Texture\Image\StadiumInflatableCactusD.dds',
    'Stadium\Media\Texture\Image\StadiumInflatableCastleD.dds',
    'Stadium\Media\Texture\Image\StadiumInflatableD.dds',
    'Stadium\Media\Texture\Image\StadiumInflatablePalmTreeD.dds',
    'Stadium\Media\Texture\Image\StadiumInflatableSnowTreeD.dds',
    'Stadium\Media\Texture\Image\StadiumPillarD.dds',
    'Stadium\Media\Texture\Image\StadiumPlatformD.dds',
    'Stadium\Media\Texture\Image\StadiumPlatformFloorD.dds',
    'Stadium\Media\Texture\Image\StadiumPubNvidiaWorldcup.dds',
    'Stadium\Media\Texture\Image\StadiumRaceSignsRubberD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadBorderD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadCircuitBorderD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadDetailsD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadDirtToRoadD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadFreeWheelingD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadGridD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadO2.dds',
    'Stadium\Media\Texture\Image\StadiumRoadRaceD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadToDirtD.dds',
    'Stadium\Media\Texture\Image\StadiumRoadTurboD.dds',
    'Stadium\Media\Texture\Image\StadiumSculpt2D.dds',
    'Stadium\Media\Texture\Image\StadiumSculptD.dds',
    'Stadium\Media\Texture\Image\StadiumStartLogoD.DDS',
    'Stadium\Media\Texture\Image\StadiumStartSignD.dds',
    'Stadium\Media\Texture\Image\StadiumStartSignGlow.dds',
    'Stadium\Media\Texture\Image\StadiumStructureAlphaD.dds',
    'Stadium\Media\Texture\Image\StadiumStructureGenericD.dds',
    'Stadium\Media\Texture\Image\StadiumWarpAuvent2D.dds',
    'Stadium\Media\Texture\Image\StadiumWarpAuventAlphaD.dds',
    'Stadium\Media\Texture\Image\StadiumWarpAuventD.dds',
    'Stadium\Media\Texture\Image\StadiumWarpLogosD.dds',
    'Stadium\Media\Texture\Image\StadiumWarpSpotsD.dds',
    'Stadium\Media\Texture\Image\StadiumWarpSpotsGlow.dds',
    'Stadium\Media\Texture\Image\StadiumWarpStands2D.dds',
    'Stadium\Media\Texture\Image\StadiumWarpStandsD.dds',
    'Vehicles\Media\Texture\Image\StadiumCarDetails.dds',
    'Vehicles\Media\Texture\Image\StadiumCarSkin.dds',
    'Vehicles\Media\Texture\Image\StadiumCarPilot.dds',
    'Interface\Media\Font\Led_Textures\Led_00.dds',
    'Interface\Media\Audio\Sound\Wav\RaceCheckPoint.wav',
    'Interface\Media\Audio\Sound\Wav\RaceFinish.wav',
    'Interface\Media\Audio\Sound\Wav\Race1.wav',
    'Interface\Media\Audio\Sound\Wav\Race2.wav',
    'Interface\Media\Audio\Sound\Wav\Race3.wav',
    'Interface\Media\Audio\Sound\Wav\RaceGo.wav',
    'Vehicles\Media\Audio\Sound\WavData\SportCarEngineIdle.wav',
    'Vehicles\Media\Audio\Sound\WavData\SportCarEngineLow.wav',
    'Vehicles\Media\Audio\Sound\WavData\SportCarEngineMid.wav',
    'Vehicles\Media\Audio\Sound\WavData\SportCarEngineFast.wav',
    'Vehicles\Media\Audio\Sound\WavData\SportCarEngineRev.wav',
    'Vehicles\Media\Audio\Sound\WavData\SportCarEngineRev2.wav'
)) {
    $source = Join-Path $gameDataSourceRoot $relativePath
    $deviceRelativePath = $relativePath.Replace('\', '/')
    $deviceDirectory = "$gameDataDestination/$($deviceRelativePath.Substring(0, $deviceRelativePath.LastIndexOf('/')))"
    & $adb -s $Serial shell mkdir -p $deviceDirectory
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to create Android content directory for $relativePath on $Serial"
    }
    & $adb -s $Serial push $source "$gameDataDestination/$deviceRelativePath"
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to deploy $relativePath to $Serial"
    }
}
$nationsSourceRoot = Join-Path $gameDataSourceRoot 'Tracks\Campaigns\Nations'
$nationsDestination = "$gameDataDestination/Tracks/Campaigns/Nations"
$campaignFiles = @(
    Get-Item -LiteralPath (Join-Path $nationsSourceRoot 'Nations.ConstructionCampaign.Gbx')
    Get-ChildItem -LiteralPath $nationsSourceRoot -Recurse -File -Filter '*.Challenge.Gbx'
)
foreach ($file in $campaignFiles) {
    $relativePath = [IO.Path]::GetRelativePath($nationsSourceRoot, $file.FullName).Replace('\', '/')
    $separator = $relativePath.LastIndexOf('/')
    $directory = if ($separator -lt 0) { $nationsDestination } else {
        "$nationsDestination/$($relativePath.Substring(0, $separator))"
    }
    & $adb -s $Serial shell mkdir -p $directory
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to create Android campaign directory for $relativePath on $Serial"
    }
    & $adb -s $Serial push $file.FullName "$nationsDestination/$relativePath"
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to deploy $relativePath to $Serial"
    }
}
Write-Output "Reference Packs, 79 campaign/runtime assets, and 65 ordered Nations challenges staged in Download/TmNationsForever on ${Serial}"
Write-Output "Launch the app, tap Select game folder, and choose Download/TmNationsForever."

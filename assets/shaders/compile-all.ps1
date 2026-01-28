Write-Host ""
Write-Host "--- Vulkan Shader Compilation ---"
Write-Host ""

Write-Host "- Ensuring .\spirv directory exists ..."
$null = New-Item -ItemType Directory -Force -Path .\spirv

# The "SLANG" environment variable should point to the /bin folder which contains the executable.
$compiler = "$($env:VULKAN_SDK)\Bin\slangc.exe"

Write-Host "- Compiling shaders to SPIR-V using: $compiler"
Write-Host "- Compiling shaders ..."
Write-Host ""

Get-ChildItem -Path .\ -include ("*.slang") -Recurse -File | ForEach-Object {
    Write-Host "... $($_.Name)"
    $params = $_.Name, "-profile", "glsl_450", "-target", "spirv", "-o", ".\spirv\$($_.BaseName).spv"
    & $compiler $params
}

Write-Host "... Done!"
Write-Host ""
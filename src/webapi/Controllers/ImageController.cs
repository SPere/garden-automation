using System.Text.Json;
using Microsoft.AspNetCore.Mvc;

namespace GardenApi.Controllers;

[ApiController]
public class ImageController(IConfiguration config) : ControllerBase
{
    private string GetBasePath() =>
        Environment.GetEnvironmentVariable("IMAGE_STORAGE_PATH")
            ?? config["ImageStorage:Path"]
            ?? "./images";

    [HttpGet("/images")]
    public IActionResult GetImages()
    {
        var basePath = GetBasePath();

        var files = Directory.Exists(basePath)
            ? new DirectoryInfo(basePath)
                .EnumerateFiles("*", SearchOption.AllDirectories)
                .OrderByDescending(f => f.LastWriteTimeUtc)
                .Take(25)
                .Select(f => new
                {
                    path = Path.GetRelativePath(basePath, f.FullName).Replace('\\', '/'),
                    size = f.Length,
                    lastModified = f.LastWriteTimeUtc
                })
            : [];

        return Ok(files);
    }

    [HttpPost("/save-image")]
    [Consumes("multipart/form-data")]
    [RequestSizeLimit(52_428_800)] // 50 MB
    public async Task<IActionResult> SaveImage([FromForm] IFormFile image, [FromForm] string metadata)
    {
        using var doc = JsonDocument.Parse(metadata);
        var source = doc.RootElement.GetProperty("source").GetString() ?? "unknown";
        // Strip characters unsafe for filenames to prevent path traversal
        var safeName = string.Concat(source.Where(c => char.IsLetterOrDigit(c) || c == '_' || c == '-'));

        var basePath = GetBasePath();

        var now = DateTime.UtcNow;
        var subFolder = now.ToString("yyMMdd");
        var ext = Path.GetExtension(image.FileName);
        var fileName = $"{now:HHmmss}_{safeName}{ext}";

        var dirPath = Path.Combine(basePath, subFolder);
        Directory.CreateDirectory(dirPath);

        await using var stream = System.IO.File.Create(Path.Combine(dirPath, fileName));
        await image.CopyToAsync(stream);

        return Ok(new { path = $"{subFolder}/{fileName}" });
    }
}

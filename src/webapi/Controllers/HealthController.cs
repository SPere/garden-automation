using Microsoft.AspNetCore.Mvc;

namespace GardenApi.Controllers;

[ApiController]
public class HealthController : ControllerBase
{
    [HttpGet("/healthz")]
    public IActionResult Health() => Ok(new { status = "ok" });
}

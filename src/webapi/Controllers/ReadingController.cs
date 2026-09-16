using GardenApi.Data;
using GardenApi.Models;
using Microsoft.AspNetCore.Mvc;

namespace GardenApi.Controllers;

[ApiController]
public class ReadingController(AppDbContext db) : ControllerBase
{
    [HttpPost("/save-reading")]
    public async Task<IActionResult> SaveReading([FromBody] SensorReadingDto dto)
    {
        db.SensorReadings.Add(new SensorReading
        {
            Temp = dto.Temp,
            Hum = dto.Hum,
            Soil1 = dto.Soil1,
            Soil2 = dto.Soil2,
            Soil3 = dto.Soil3,
            Soil4 = dto.Soil4,
            Light = dto.Light,
            Source = dto.Source,
            CreatedAt = DateTime.UtcNow
        });

        await db.SaveChangesAsync();
        return Accepted();
    }
}

public record SensorReadingDto(
    double Temp,
    double Hum,
    double Soil1,
    double Soil2,
    double Soil3,
    double Soil4,
    double Light,
    string Source
);

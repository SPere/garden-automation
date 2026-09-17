using System.Globalization;
using GardenApi.Data;
using GardenApi.Models;
using Microsoft.AspNetCore.Mvc;

namespace GardenApi.Controllers;

[ApiController]
public class ReadingController(AppDbContext db) : ControllerBase
{
    // Expected timestamp format: yyyyddMMHHmmssfff  e.g. 20261709100610123
    private const string TimestampFormat = "yyyyddMMHHmmssfff";

    [HttpPost("/save-reading")]
    public async Task<IActionResult> SaveReading([FromBody] DeviceReadingDto dto)
    {
        var timestamp = ParseTimestamp(dto.Timestamp);

        foreach (var r in dto.Readings)
        {
            if (double.TryParse(r.Value, NumberStyles.Float, CultureInfo.InvariantCulture, out var numericValue))
            {
                db.Metrics.Add(new Metric
                {
                    Device = dto.Device,
                    Sensor = r.Sensor,
                    Type = r.Type,
                    Reading = numericValue,
                    Timestamp = timestamp
                });
            }
            else
            {
                db.Logs.Add(new SensorLog
                {
                    Device = dto.Device,
                    Sensor = r.Sensor,
                    Type = r.Type,
                    Reading = r.Value,
                    Timestamp = timestamp
                });
            }
        }

        await db.SaveChangesAsync();
        return Accepted();
    }

    private static DateTime ParseTimestamp(string? timestamp)
    {
        if (string.IsNullOrWhiteSpace(timestamp))
            return DateTime.UtcNow;

        if (DateTime.TryParseExact(timestamp, TimestampFormat,
                CultureInfo.InvariantCulture, DateTimeStyles.None, out var parsed))
            return parsed;

        return DateTime.UtcNow;
    }
}

public record DeviceReadingDto(
    string Device,
    string? Timestamp,
    IReadOnlyList<ReadingItemDto> Readings
);

public record ReadingItemDto(
    string Sensor,
    string Type,
    string Value
);

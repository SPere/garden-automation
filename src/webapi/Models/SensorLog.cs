using System.ComponentModel.DataAnnotations;

namespace GardenApi.Models;

public class SensorLog
{
    [Key]
    public int Id { get; set; }
    public string Device { get; set; } = string.Empty;
    public string Sensor { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty;
    public string Reading { get; set; } = string.Empty;
    public DateTime Timestamp { get; set; }
}

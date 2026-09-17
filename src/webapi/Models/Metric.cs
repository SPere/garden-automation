using System.ComponentModel.DataAnnotations;

namespace GardenApi.Models;

public class Metric
{
    [Key]
    public int Id { get; set; }
    public string Device { get; set; } = string.Empty;
    public string Sensor { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty;
    public double Reading { get; set; }
    public DateTime Timestamp { get; set; }
}

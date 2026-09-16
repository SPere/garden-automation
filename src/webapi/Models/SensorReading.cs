using System.ComponentModel.DataAnnotations;

namespace GardenApi.Models;

public class SensorReading
{
    [Key]
    public int Id { get; set; }
    public double Temp { get; set; }
    public double Hum { get; set; }
    public double Soil1 { get; set; }
    public double Soil2 { get; set; }
    public double Soil3 { get; set; }
    public double Soil4 { get; set; }
    public double Light { get; set; }
    public string Source { get; set; } = string.Empty;
    public DateTime CreatedAt { get; set; }
}

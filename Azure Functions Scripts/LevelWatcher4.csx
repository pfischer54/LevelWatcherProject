#r "Newtonsoft.Json"
#r "Microsoft.WindowsAzure.Storage"

using System;
using System.Net;
using Newtonsoft.Json;
using System.Collections.Generic;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;

public static async Task<object> Run(HttpRequestMessage req, TraceWriter log, CloudTable tableBinding)
{
    log.Info($"Webhook was triggered!");

    string jsonContent = await req.Content.ReadAsStringAsync(); //removed await
        log.Info("jsonContent: " + jsonContent);

//Set json converter datetime conversion options to treat field as a string (I.e. do not try and parse datetimes) so that we do not loose millisecond granularity.  We need this so as to create unique index as we use this as the index for the RowKey.
	var settings = new JsonSerializerSettings
			{
				DateParseHandling = 0
			};

    dynamic data = JsonConvert.DeserializeObject(jsonContent, settings);
 
//Persist to table data
 LevelMeasurementData levelMeasurementData = new LevelMeasurementData() { 
                    PartitionKey = data.source, 
                    RowKey = data.RowKey, 
                    SensorId = data.SensorId,
                    SignalStrength = data.SS,
                    LevelBits = data.LsBits,
                    ZeroingInProgress = data.ZeroingInProgress,
                     Temperature = data.T,
                     Pressure = data.P
                     } ;                 

 TableOperation insertOrReplaceOperation = TableOperation.InsertOrReplace(levelMeasurementData);
tableBinding.Execute(insertOrReplaceOperation);


    return req.CreateResponse(HttpStatusCode.OK, new
    {
        DataEcho = $"Data: LevelMeasurementData"
    });
}
public class LevelMeasurementData : TableEntity
{
  public LevelMeasurementData(string partitionKey, string rowKey)
    {
        this.PartitionKey = partitionKey;
        this.RowKey = rowKey;
    }
    
    public LevelMeasurementData() { }

    public string Source { get; set; }
    public string SensorId {get; set; }
    public string SignalStrength { get; set; }
    public string LevelBits{ get; set; }
    public string ZeroingInProgress{ get; set; }
    public string Temperature{ get; set; }
    public string Pressure{ get; set; }
}


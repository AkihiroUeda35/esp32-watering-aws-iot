import json
import boto3
from decimal import Decimal

dynamoDB = boto3.resource("dynamodb")
table = dynamoDB.Table("esp32") 

def lambda_handler(event, context):
    print(event)
    print(context)
    # Input Data from esp32 to dynamo table
    table.put_item(
      Item = {
        "device_id": event["deviceid"],
        "timestamp": event["timestamp"], 
        "temperature": Decimal(str(event["temperature"])),
        "humidity": Decimal(str(event["humidity"])),
        "moisture": Decimal(str(event["moisture"]))
      }
    )

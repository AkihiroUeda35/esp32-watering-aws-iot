# Notify with Line Notify when the device's battery runs out detected by dynamo db.
# Called once a day by Cloudwatch.
import urllib.request
import boto3
from datetime import datetime, timedelta, timezone
from boto3.dynamodb.conditions import Key, Attr
from zoneinfo import ZoneInfo
import json

from botocore.exceptions import ClientError

def get_token():
    """
    Get Line Notify Token from Secret Manager
    """
    secret_name = "line_notify"
    region_name = "ap-northeast-1"

    # Create a Secrets Manager client
    session = boto3.session.Session()
    client = session.client(
        service_name='secretsmanager',
        region_name=region_name
    )

    try:
        get_secret_value_response = client.get_secret_value(
            SecretId=secret_name
        )
    except ClientError as e:
        # For a list of exceptions thrown, see
        # https://docs.aws.amazon.com/secretsmanager/latest/apireference/API_GetSecretValue.html
        raise e
    secret = json.loads(get_secret_value_response['SecretString'])
    return secret["token"]

dynamoDB = boto3.resource("dynamodb")
table = dynamoDB.Table("esp32") # DynamoDB Table name of ESP32 data

api_url = 'https://notify-api.line.me/api/notify'
contents = '電池が切れたよー'

request_headers = {
    'Content-Type': 'application/x-www-form-urlencoded',
    'Authorization': 'Bearer' + ' ' + get_token()
}

def lambda_handler(event, context):
    # Get this 3 hours of data from dynamo db
    ts = (datetime.now(ZoneInfo("Asia/Tokyo")) - timedelta(hours=3)).isoformat()
    result = table.query(KeyConditionExpression = Key("device_id").eq("esp32") & Key('timestamp').gt(ts))

    # Convert to Bytes
    data = urllib.parse.urlencode( {'message': contents}).encode('ascii')
    req = urllib.request.Request(api_url, headers=request_headers, data=data, method='POST')
    print("Check result...", len(result["Items"]))
    # if no item found, notify with line.
    if len(result["Items"]) == 0: 
        print("No Items!")
        conn = urllib.request.urlopen(req)
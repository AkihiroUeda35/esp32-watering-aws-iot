# esp32-watering-aws-iot

Watering with IoT using [WayinTop Watering tool](https://github.com/WayinTop/Automatic-Plant-Watering-System-Tutorial) and AWS IoT.

## Build and Write Firmware

Rename secrets_template.h and edit Wi-Fi SSID, Wi-Fi password and AWS IoT keys that is got from AWS IoT console.

Build and run with Arduino-IDE.

## Create DynamoDB

Create DynamoDB with following conditions.

- Table name: `esp32`
- Primary key: `device_id`
- Sort key: `timestamp`

## Deploy Lambda

- watering.py
    - Script to get device's data and save it to DynamoDB.
    - Need access to DynamoDB.
    - Triggered by AWS IoT.

- line_notify.py
    - Script to check recent 3 hours data from DynamoDB and notify if there is not data via Line Notify.
    - Need access to Secret Manager to get Line Notify Token.
    - Triggered by EventBridge once a day trigger.

## Set Line Notify Token

Get Line Notify token and save at Secret Manager and allow access to lambda.

```python
    secret_name = "line_notify"
    region_name = "ap-northeast-1"
```

## Set trigger for line_notify.py

EventBridge Event with Cron formula like below.

```txt
0 23 * * ? *
```

## Allow access to Lambda 

Set AWS IAM Role and policy.
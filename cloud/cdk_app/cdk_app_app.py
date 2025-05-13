#!/usr/bin/env python3
from aws_cdk import core
from flex_sensor_stack import FlexSensorStack

# Create the CDK app
app = core.App()

# Define the stack with custom configurations
FlexSensorStack(app, "FlexSensorStack")

# Synthesize the app to generate CloudFormation templates
app.synth()
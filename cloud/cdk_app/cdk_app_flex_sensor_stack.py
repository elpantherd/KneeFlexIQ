from aws_cdk import (
    core as cdk,
    aws_s3 as s3,
    aws_lambda as _lambda,
    aws_apigateway as apigateway,
    aws_iam as iam,
    aws_s3_notifications as s3n,
)

class FlexSensorStack(cdk.Stack):
    def __init__(self, scope: cdk.Construct, id: str, **kwargs) -> None:
        super().__init__(scope, id, **kwargs)

        # Create S3 bucket for data storage
        bucket = s3.Bucket(self, "FlexSensorDataBucket",
            bucket_name='flex-sensor-bucket-12345',  # Replace with a unique name
            versioned=True,
            removal_policy=cdk.RemovalPolicy.DESTROY
        )
        cdk.CfnOutput(self, "BucketName", value=bucket.bucket_name)

        # Create Lambda function for API
        lambda_fn = _lambda.Function(self, "FlexSensorLambda",
            runtime=_lambda.Runtime.PYTHON_3_8,
            handler="index.lambda_handler",
            code=_lambda.Code.from_asset("lambda"),
            environment={"ENDPOINT_NAME": "your-endpoint-name"}  # Replace with actual endpoint
        )

        # Add IAM policy for SageMaker endpoint invocation
        lambda_fn.add_to_role_policy(iam.PolicyStatement(
            actions=["sagemaker:InvokeEndpoint"],
            resources=[f"arn:aws:sagemaker:*:*:endpoint/your-endpoint-name"]
        ))

        # Create API Gateway for Lambda
        api = apigateway.LambdaRestApi(self, "FlexSensorApi",
            handler=lambda_fn,
            proxy=False
        )
        flex_resource = api.root.add_resource("flex")
        flex_resource.add_method("POST")

        # Add S3 notification for new data uploads
        notification_lambda = _lambda.Function(self, "S3NotificationLambda",
            runtime=_lambda.Runtime.PYTHON_3_8,
            handler="notification.handler",
            code=_lambda.Code.from_asset("lambda"),
        )
        bucket.add_event_notification(s3.EventType.OBJECT_CREATED, s3n.LambdaDestination(notification_lambda))

        # Output API URL
        cdk.CfnOutput(self, "ApiUrl", value=api.url)
import json
import boto3
import logging
import time

# Configure logging for detailed debugging and monitoring
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# SageMaker runtime client
runtime = boto3.client('runtime.sagemaker')
ENDPOINT_NAME = 'your-endpoint-name'  # Replace with actual endpoint name

def invoke_endpoint_with_retry(flex_value, retries=3):
    """
    Invokes the SageMaker endpoint with retry logic.
    
    Args:
        flex_value (float): Flex sensor value to classify.
        retries (int): Number of retry attempts.
    
    Returns:
        str: Classification result.
    """
    attempt = 0
    while attempt < retries:
        try:
            response = runtime.invoke_endpoint(
                EndpointName=ENDPOINT_NAME,
                ContentType='text/csv',
                Body=str(flex_value)
            )
            result = json.loads(response['Body'].read().decode())
            return result
        except Exception as e:
            attempt += 1
            logger.warning(f"Endpoint invocation attempt {attempt} failed: {str(e)}")
            time.sleep(2 ** attempt)  # Exponential backoff
    raise RuntimeError(f"Failed to invoke endpoint after {retries} attempts")

def lambda_handler(event, context):
    """
    AWS Lambda handler function to classify flex sensor data.
    
    Args:
        event (dict): Event data containing 'flex_value'.
        context: Lambda context object.
    
    Returns:
        dict: Response with classification result or error message.
    """
    try:
        logger.info(f"Received event: {json.dumps(event)}")
        
        if 'flex_value' not in event:
            raise ValueError("Missing 'flex_value' in event")
        flex_value = event['flex_value']
        if not isinstance(flex_value, (int, float)):
            raise ValueError("flex_value must be a number")
        
        result = invoke_endpoint_with_retry(flex_value)
        
        response = {
            'statusCode': 200,
            'body': json.dumps({'classification': result})
        }
        logger.info(f"Response: {response}")
        return response
    except ValueError as e:
        logger.error(f"Input validation error: {str(e)}")
        return {
            'statusCode': 400,
            'body': json.dumps({'error': str(e)})
        }
    except RuntimeError as e:
        logger.error(f"Endpoint invocation error: {str(e)}")
        return {
            'statusCode': 500,
            'body': json.dumps({'error': str(e)})
        }
    except Exception as e:
        logger.error(f"Unexpected error in Lambda: {str(e)}")
        return {
            'statusCode': 500,
            'body': json.dumps({'error': 'Internal server error'})
        }
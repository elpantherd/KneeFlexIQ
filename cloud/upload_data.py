import boto3
import os
import logging
import time

# Configure logging for detailed debugging and monitoring
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Custom exception for upload-related issues
class UploadError(Exception):
    """Custom exception for upload failures."""
    pass

def check_file_exists(file_path):
    """
    Check if the file exists at the specified path.
    
    Args:
        file_path (str): Path to the file.
    
    Raises:
        UploadError: If the file does not exist.
    """
    if not os.path.exists(file_path):
        raise UploadError(f"File not found: {file_path}")
    logger.info(f"File exists: {file_path}")

def upload_to_s3(file_path, bucket_name, s3_key, retries=3):
    """
    Upload a file to S3 with retry logic.
    
    Args:
        file_path (str): Path to the file to upload.
        bucket_name (str): Name of the S3 bucket.
        s3_key (str): Key to store the file under in S3.
        retries (int): Number of retry attempts.
    
    Raises:
        UploadError: If upload fails after retries.
    """
    s3_client = boto3.client('s3')
    attempt = 0
    while attempt < retries:
        try:
            s3_client.upload_file(file_path, bucket_name, s3_key)
            logger.info(f"Successfully uploaded {file_path} to s3://{bucket_name}/{s3_key}")
            return
        except Exception as e:
            attempt += 1
            logger.warning(f"Upload attempt {attempt} failed: {str(e)}")
            time.sleep(2 ** attempt)  # Exponential backoff
    raise UploadError(f"Failed to upload {file_path} after {retries} attempts")

def create_bucket_if_not_exists(bucket_name):
    """
    Creates an S3 bucket if it does not exist.
    
    Args:
        bucket_name (str): Name of the S3 bucket.
    """
    s3_client = boto3.client('s3')
    try:
        s3_client.head_bucket(Bucket=bucket_name)
        logger.info(f"Bucket {bucket_name} already exists")
    except s3_client.exceptions.ClientError:
        s3_client.create_bucket(Bucket=bucket_name)
        logger.info(f"Created bucket {bucket_name}")

def main():
    """
    Main function to upload training data to S3.
    """
    try:
        file_path = 'train.csv'
        bucket_name = 'flex-sensor-bucket-12345'  # Replace with a unique name
        s3_key = 'data/train.csv'
        
        logger.info("Starting data upload process...")
        
        # Validate and check file
        check_file_exists(file_path)
        
        # Ensure bucket exists
        create_bucket_if_not_exists(bucket_name)
        
        # Upload file with retries
        upload_to_s3(file_path, bucket_name, s3_key)
        
        logger.info("Data upload completed successfully")
    except UploadError as e:
        logger.error(f"Upload error: {str(e)}")
        raise
    except Exception as e:
        logger.error(f"Unexpected error during upload: {str(e)}")
        raise

if __name__ == '__main__':
    main()
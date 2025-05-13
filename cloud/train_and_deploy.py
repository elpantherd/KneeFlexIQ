import boto3
import sagemaker
from sagemaker.sklearn.estimator import SKLearn
from sagemaker.tuner import HyperparameterTuner, IntegerParameter
import logging

# Configure logging for detailed debugging and monitoring
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

def get_sagemaker_role():
    """
    Retrieves the SageMaker execution role.
    
    Returns:
        str: ARN of the SageMaker execution role.
    """
    try:
        role = sagemaker.get_execution_role()
        logger.info(f"Using SageMaker role: {role}")
        return role
    except Exception as e:
        logger.error(f"Failed to get SageMaker role: {str(e)}")
        raise

def setup_sagemaker_session():
    """
    Sets up a SageMaker session.
    
    Returns:
        sagemaker.Session: SageMaker session object.
    """
    try:
        sess = sagemaker.Session()
        logger.info("SageMaker session created successfully")
        return sess
    except Exception as e:
        logger.error(f"Failed to create SageMaker session: {str(e)}")
        raise

def train_model(role, sess, bucket, prefix):
    """
    Trains the model using SageMaker.
    
    Args:
        role (str): SageMaker execution role.
        sess (sagemaker.Session): SageMaker session.
        bucket (str): S3 bucket name.
        prefix (str): S3 prefix for training data.
    """
    try:
        train_data = f's3://{bucket}/{prefix}/train.csv'
        logger.info(f"Training data location: {train_data}")
        
        sklearn_estimator = SKLearn(
            entry_point='train.py',
            role=role,
            instance_count=1,
            instance_type='ml.m5.xlarge',
            framework_version='0.23-1',
            py_version='py3',
            hyperparameters={'n_estimators': 100, 'random_state': 42}
        )
        logger.info("SageMaker SKLearn estimator created")
        
        sklearn_estimator.fit({'train': train_data})
        logger.info("Model training completed")
        
        return sklearn_estimator
    except Exception as e:
        logger.error(f"Error during model training: {str(e)}")
        raise

def deploy_model(estimator, sess):
    """
    Deploys the trained model as a SageMaker endpoint.
    
    Args:
        estimator (sagemaker.estimator.Estimator): Trained SageMaker estimator.
        sess (sagemaker.Session): SageMaker session.
    
    Returns:
        sagemaker.Predictor: SageMaker predictor for the deployed model.
    """
    try:
        predictor = estimator.deploy(initial_instance_count=1, instance_type='ml.m5.xlarge')
        logger.info(f"Model deployed at endpoint: {predictor.endpoint_name}")
        return predictor
    except Exception as e:
        logger.error(f"Error during model deployment: {str(e)}")
        raise

def main():
    """
    Main function to train and deploy the model using SageMaker.
    """
    try:
        role = get_sagemaker_role()
        sess = setup_sagemaker_session()
        
        bucket = 'flex-sensor-bucket-12345'  # Replace with your bucket name
        prefix = 'data'
        
        estimator = train_model(role, sess, bucket, prefix)
        predictor = deploy_model(estimator, sess)
        
        logger.info("Training and deployment completed successfully")
    except Exception as e:
        logger.error(f"Error in main execution: {str(e)}")
        raise

if __name__ == '__main__':
    main()
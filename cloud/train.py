import argparse
import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.metrics import accuracy_score, classification_report
from sklearn.preprocessing import StandardScaler
import joblib
import logging
import os
import sys
import numpy as np

# Configure logging for detailed debugging and monitoring
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Custom exception for data-related issues
class DataValidationError(Exception):
    """Custom exception for data validation failures."""
    pass

def validate_data(df):
    """
    Validates the input DataFrame for required columns and sufficient data.
    
    Args:
        df (pd.DataFrame): Input DataFrame to validate
    
    Raises:
        DataValidationError: If validation fails
    """
    logger.info("Validating input data...")
    if df.empty:
        raise DataValidationError("Input DataFrame is empty")
    
    required_columns = ['flex_value', 'label']
    missing_columns = [col for col in required_columns if col not in df.columns]
    if missing_columns:
        raise DataValidationError(f"Missing required columns: {missing_columns}")
    
    if len(df) < 10:
        raise DataValidationError("Insufficient data: less than 10 rows")
    
    if df['flex_value'].isnull().any() or df['label'].isnull().any():
        raise DataValidationError("Data contains null values")
    
    logger.info("Data validation passed successfully")

def preprocess_data(df):
    """
    Preprocesses the input data by scaling features.
    
    Args:
        df (pd.DataFrame): Input DataFrame
    
    Returns:
        pd.DataFrame: Preprocessed DataFrame
    """
    logger.info("Starting data preprocessing...")
    scaler = StandardScaler()
    df['flex_value_scaled'] = scaler.fit_transform(df[['flex_value']])
    logger.info("Data preprocessing completed")
    return df

def split_features_labels(df):
    """
    Splits DataFrame into features (X) and labels (y).
    
    Args:
        df (pd.DataFrame): Input DataFrame
    
    Returns:
        tuple: (X, y) where X is features and y is labels
    """
    X = df[['flex_value_scaled']]
    y = df['label']
    return X, y

def train_model(train_data_path):
    """
    Trains a Random Forest Classifier on the provided training data.
    
    Args:
        train_data_path (str): Path to the training CSV file
    
    Raises:
        Exception: If training fails
    """
    try:
        logger.info(f"Starting model training with data from {train_data_path}")
        df = pd.read_csv(train_data_path)
        logger.info(f"Loaded {len(df)} rows of training data")
        
        validate_data(df)
        df = preprocess_data(df)
        X, y = split_features_labels(df)
        
        logger.info(f"Feature matrix shape: {X.shape}")
        logger.info(f"Label vector shape: {y.shape}")
        
        X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
        logger.info(f"Training set size: {len(X_train)}, Test set size: {len(X_test)}")
        
        model = RandomForestClassifier(n_estimators=100, max_depth=10, min_samples_split=5, random_state=42)
        logger.info("Random Forest Classifier initialized")
        
        cv_scores = cross_val_score(model, X_train, y_train, cv=5)
        logger.info(f"Cross-validation scores: {cv_scores}")
        logger.info(f"Mean CV score: {np.mean(cv_scores):.3f}")
        
        model.fit(X_train, y_train)
        logger.info("Model training completed")
        
        y_pred = model.predict(X_test)
        accuracy = accuracy_score(y_test, y_pred)
        logger.info(f"Test set accuracy: {accuracy:.3f}")
        
        report = classification_report(y_test, y_pred)
        logger.info(f"Classification report:\n{report}")
        
        output_dir = '/opt/ml/model'
        os.makedirs(output_dir, exist_ok=True)
        model_path = os.path.join(output_dir, 'model.joblib')
        joblib.dump(model, model_path)
        logger.info(f"Model saved to {model_path}")
        
    except DataValidationError as e:
        logger.error(f"Data validation error: {str(e)}")
        raise
    except Exception as e:
        logger.error(f"Unexpected error during training: {str(e)}")
        raise

def test_model_training():
    """
    Simulated unit test for the train_model function.
    """
    logger.info("Running unit tests for model training...")
    test_df = pd.DataFrame({
        'flex_value': [100, 200, 300, 400, 500],
        'label': ['Low', 'Low', 'Medium', 'High', 'High']
    })
    test_path = 'test_train.csv'
    test_df.to_csv(test_path, index=False)
    
    try:
        train_model(test_path)
        logger.info("Unit test passed: Model trained successfully")
    except Exception as e:
        logger.error(f"Unit test failed: {str(e)}")
    finally:
        if os.path.exists(test_path):
            os.remove(test_path)
            logger.info("Cleaned up test file")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Train a Random Forest model for flex sensor data")
    parser.add_argument('--train', type=str, required=True, help="Path to the training data CSV file")
    args = parser.parse_args()
    
    logger.info("Script execution started")
    train_model(args.train)
    logger.info("Script execution completed successfully")
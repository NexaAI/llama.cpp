from huggingface_hub import HfApi, login
import os
import sys

def upload_gguf_model(model_path, repo_id):
    """
    Upload a GGUF model to HuggingFace Hub
    
    Args:
        model_path (str): Local path to the GGUF model file
        repo_id (str): HuggingFace repository ID (format: username/repo-name)
        token (str): HuggingFace API token
    """
    try:
        # Login to Hugging Face
        login()
        
        # Initialize the HF API
        api = HfApi()
        
        # Get the filename from the path
        model_filename = os.path.basename(model_path)
        
        print(f"Starting upload of {model_filename} to {repo_id}...")
        
        # Upload the file
        api.upload_file(
            path_or_fileobj=model_path,
            path_in_repo=model_filename,
            repo_id=repo_id,
            repo_type="model"
        )
        
        print(f"Successfully uploaded {model_filename} to {repo_id}")
        
    except Exception as e:
        print(f"Error uploading model: {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    # Replace these values with your actual paths and token
    MODEL_PATH = "qwen2/Qwen2-7.8B-F16.gguf"  # Local path to your GGUF model
    # MODEL_PATH = "nano-omni-instruct.mel-filters-audio_tower-multi_modal_projector.gguf"  # Local path to your GGUF model
    REPO_ID = "nexa-collaboration/nano-omini-instruct-gguf"     # Your repository ID
    
    upload_gguf_model(MODEL_PATH, REPO_ID)
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
    REPO_ID = "nexa-collaboration/qwen2-audio-7B-instruct-gguf"
    MODEL_PATH = "qwen2/Qwen2-7.8B-F16.gguf"  # Local path to your GGUF model
    upload_gguf_model(MODEL_PATH, REPO_ID)
    MODEL_PATH = "qwen2-audio-instruct.mel-filters-audio_tower-multi_modal_projector.gguf"  # Local path to your GGUF model
    upload_gguf_model(MODEL_PATH, REPO_ID)
from transformers import Qwen2AudioForConditionalGeneration, AutoProcessor

def load_nexa_model_and_save_local(model_name, processor_name):
    full_model = Qwen2AudioForConditionalGeneration.from_pretrained(model_name)
    processor = AutoProcessor.from_pretrained(processor_name)
    tokenizer = processor.tokenizer
    full_model.language_model.save_pretrained("qwen2/")
    tokenizer.save_pretrained("qwen2/")

if __name__ == "__main__":
    # Model directory and output filenames
    load_nexa_model_and_save_local(model_name="Qwen/Qwen2-Audio-7B-Instruct", processor_name="Qwen/Qwen2-Audio-7B-Instruct")

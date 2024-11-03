import torch

import sys
sys.path.append("../../../huggingface")
from nexa_omni_audio.modeling_gemma2_audio import Gemma2AudioForConditionalGeneration
from nexa_omni_audio.processing_gemma2_audio import Gemma2AudioProcessor


if __name__ == "__main__":

    local_model: Gemma2AudioForConditionalGeneration = Gemma2AudioForConditionalGeneration.from_pretrained("nexa-collaboration/nano-omini-instruct", device_map="cuda", torch_dtype=torch.bfloat16)
    processor: Gemma2AudioProcessor = Gemma2AudioProcessor.from_pretrained("nexa-collaboration/gemma2-2b-audio-whisper-medium-full-model-it")

    gemma2_save_dir = "./models/nano-omini-instruct-gemma2"

    print("Saving gemma2 model to ", gemma2_save_dir)
    gemma2_model = local_model.language_model
    gemma2_model.save_pretrained(gemma2_save_dir)

    print("Saving tokenizer to ", gemma2_save_dir)
    tokenizer = processor.tokenizer
    tokenizer.save_pretrained(gemma2_save_dir)

    print("Done!")

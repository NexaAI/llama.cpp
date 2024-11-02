import os
import numpy as np
from transformers import Gemma2ForCausalLM
from gguf import GGUFWriter, GGUFReader
import sys
sys.path.append("../../../huggingface")
from nexa_omni_audio.modeling_gemma2_audio import Gemma2AudioForConditionalGeneration
from nexa_omni_audio.processing_gemma2_audio import Gemma2AudioProcessor

def load_nexa_model_and_save_local(model_name = "nexa-collaboration/nano-omini-instruct", processor_name = "nexa-collaboration/gemma2-2b-audio-whisper-medium-full-model-it"):
    full_model = Gemma2AudioForConditionalGeneration.from_pretrained(model_name)
    processor = Gemma2AudioProcessor.from_pretrained(processor_name)
    tokenizer = processor.tokenizer
    full_model.language_model.save_pretrained("gemma2/")
    tokenizer.save_pretrained("gemma2/")
    language_model = Gemma2ForCausalLM.from_pretrained("gemma2/")

if __name__ == "__main__":
    # Model directory and output filenames
    dir_model = os.getcwd()
    fname_in = f"{dir_model}/2b_it_v2.gguf"
    fname_out = f"{dir_model}/nexa_omni_converted.gguf"
    load_nexa_model_and_save_local(model_name="nexa-collaboration/nano-omini-instruct", processor_name="nexa-collaboration/gemma2-2b-audio-whisper-medium-full-model-it")

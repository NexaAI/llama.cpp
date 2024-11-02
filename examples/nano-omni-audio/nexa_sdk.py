import ctypes
import logging
import os

import omni_audio_cpp


class NexaOmniAudioInference:
    """
    A class used for loading Bark text-to-speech models and running text-to-speech generation.

    Methods:
        run: Run the text-to-speech generation loop.
        audio_generation: Generate audio from the user input.

    Args:
        model_path (str): Path to the Bark model file.
        n_threads (int): Number of threads to use for processing. Defaults to 1.
        seed (int): Seed for random number generation. Defaults to 0.
        output_dir (str): Output directory for tts. Defaults to "tts".
        sampling_rate (int): Sampling rate for audio processing. Defaults to 24000.
        verbosity (int): Verbosity level for the Bark model. Defaults to 0.
    """

    def __init__(
        self, model_path: str, mmproj_path: str, n_gpu_layers: int = -1, **kwargs
    ):
        self.model_path = model_path
        self.mmproj_path = mmproj_path
        self.n_gpu_layers = n_gpu_layers

        self.ctx_params = omni_audio_cpp.omni_context_default_params()
        self.context = None
        self._load_model()

    def _load_model(self):
        logging.debug(f"Loading model from {self.model_path} and {self.mmproj_path}")
        try:
            self.ctx_params.model = ctypes.c_char_p(self.model_path.encode("utf-8"))
            self.ctx_params.mmproj = ctypes.c_char_p(self.mmproj_path.encode("utf-8"))
            self.ctx_params.n_gpu_layers = (
                0x7FFFFFFF if self.n_gpu_layers == -1 else self.n_gpu_layers
            )  # 0x7FFFFFFF is INT32 max, will be auto set to all layers

            self.context = omni_audio_cpp.omni_init_context(
                ctypes.byref(self.ctx_params)
            )
            if not self.context:
                raise RuntimeError("Failed to load Bark model")
            logging.debug("Model loaded successfully")
        except Exception as e:
            logging.error(f"Error loading model: {e}")
            raise

    def run(self):
        while True:
            try:
                audio_path = input("Audio Path (leave empty if no audio): ")
                if audio_path and not os.path.exists(audio_path):
                    print(f"'{audio_path}' is not a path to audio. Will ignore.")

                user_input = input("Enter text: ")

                self.ctx_params.file = ctypes.c_char_p(audio_path.encode("utf-8"))
                self.ctx_params.prompt = ctypes.c_char_p(user_input.encode("utf-8"))

                omni_audio_cpp.omni_process_full(
                    self.context, ctypes.byref(self.ctx_params)
                )
            except KeyboardInterrupt:
                print("\nExiting...")
                break
            except Exception as e:
                logging.error(f"\nError during audio generation: {e}", exc_info=True)

    def __del__(self):
        """
        Destructor to free the Bark context when the instance is deleted.
        """
        if self.context:
            omni_audio_cpp.omni_free(self.context)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Run audio-in text-out generation with nexa-omni-audio model"
    )
    parser.add_argument("--model", type=str, help="Path to the gemma2 model file")
    parser.add_argument("--mmproj", type=str, help="Path to the mmproj file")
    parser.add_argument(
        "-ngl",
        "--n_gpu_layers",
        type=int,
        default=-1,
        help="Number of GPU layers to use for processing",
    )

    args = parser.parse_args()

    inference = NexaOmniAudioInference(
        args.model,
        args.mmproj,
        args.n_gpu_layers,
    )
    inference.run()
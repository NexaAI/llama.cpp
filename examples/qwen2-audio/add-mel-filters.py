import struct

from gguf import GGUFReader, GGUFWriter
import numpy as np

# Constants
GGML_FILE_MAGIC = 0x67676D6C  # Example magic number, replace with actual if different

bin_to_torch = {
    "n_audio_ctx": "max_source_positions",
    "n_audio_state": "d_model",
    "n_audio_head": "encoder_attention_heads",
    "n_audio_layer": "encoder_layers",
}


def read_safe(file, fmt):
    size = struct.calcsize(fmt)
    data = file.read(size)
    return struct.unpack(fmt, data)


def write_safe(file, fmt, *data):
    file.write(struct.pack(fmt, *data))


def read_hparams_and_mels_from_bin(file):

    # verify magic with python struct
    magic = read_safe(file, "i")[0]
    if magic != GGML_FILE_MAGIC:
        raise ValueError("Invalid magic number")

    hparams = {}
    mel_filters = {}

    # read hparams
    hparams["n_vocab"] = read_safe(file, "i")[0]
    hparams["n_audio_ctx"] = read_safe(file, "i")[0]
    hparams["n_audio_state"] = read_safe(file, "i")[0]
    hparams["n_audio_head"] = read_safe(file, "i")[0]
    hparams["n_audio_layer"] = read_safe(file, "i")[0]
    hparams["n_text_ctx"] = read_safe(file, "i")[0]
    hparams["n_text_state"] = read_safe(file, "i")[0]
    hparams["n_text_head"] = read_safe(file, "i")[0]
    hparams["n_text_layer"] = read_safe(file, "i")[0]
    hparams["n_mels"] = read_safe(file, "i")[0]
    hparams["ftype"] = read_safe(file, "i")[0]

    # read mel filters
    mel_filters["n_mel"] = read_safe(file, "i")[0]
    mel_filters["n_fft"] = read_safe(file, "i")[0]

    data = read_safe(file, "f" * mel_filters["n_mel"] * mel_filters["n_fft"])
    mel_filters["data"] = np.array(data).astype(np.float32)

    return hparams, mel_filters


def read_hparams_and_tensors_from_gguf(file_path):

    reader = GGUFReader(file_path)

    hparams = {}
    tensors = {}

    # hparams
    hparam_list = bin_to_torch.values()
    for param in hparam_list:
        hparams[param] = reader.fields[param].parts[reader.fields[param].data[0]]
        hparams[param] = int(hparams[param][0])

    for tensor in reader.tensors:
        tensors[tensor.name] = tensor.data

    return hparams, tensors


def verify_hparams(hparams_bin, hparams_gguf):

    for k1, k2 in zip(bin_to_torch.keys(), bin_to_torch.values()):
        if hparams_bin[k1] != hparams_gguf[k2]:
            raise ValueError(
                f"Mismatch in {k1}: {hparams_bin[k1]} != {hparams_gguf[k2]}"
            )


def read_and_save_model(input_file_path, gguf_file_path, output_file_path):
    writer = GGUFWriter(output_file_path, "nano-omni-audio-encoder")

    with open(input_file_path, "rb") as infile:
        # Read hparams and mel filters from ggml-medium.bin
        bin_hparams, mel_filters = read_hparams_and_mels_from_bin(infile)

        # Read hparams and tensors from nano-omni-audio-encoder.gguf
        gguf_hparams, gguf_tensors = read_hparams_and_tensors_from_gguf(gguf_file_path)

        # Verify hparams are the same
        verify_hparams(bin_hparams, gguf_hparams)

        # Write hparams to output file
        for param in gguf_hparams:
            writer.add_int32(param, gguf_hparams[param])
            print(f"Writing hparam {param} = {gguf_hparams[param]}")

        # Write mel filters to output file
        writer.add_int32("n_mel", mel_filters["n_mel"])
        writer.add_int32("n_fft", mel_filters["n_fft"])
        print(f"Writing mel filters n_mel = {mel_filters['n_mel']}")
        print(f"Writing mel filters n_fft = {mel_filters['n_fft']}")

        # Write mel filters data to output file
        writer.add_tensor("mel_filters_data", mel_filters["data"])
        print(f"Writing mel filters data with shape {mel_filters['data'].shape}")

        # Write tensors to output file
        for name, tensor in gguf_tensors.items():
            writer.add_tensor(name, tensor)
            print(f"Writing tensor {name} with shape {tensor.shape}")

        writer.write_header_to_file()
        writer.write_kv_data_to_file()
        writer.write_tensors_to_file()
        writer.close()


if __name__ == "__main__":
    bin_file = "ggml-large-v3.bin"
    gguf_file = "qwen2-audio-instruct.audio_tower-multi_modal_projector.gguf"
    output_file = "qwen2-audio-instruct.mel-filters-audio_tower-multi_modal_projector.gguf"
    read_and_save_model(bin_file, gguf_file, output_file)

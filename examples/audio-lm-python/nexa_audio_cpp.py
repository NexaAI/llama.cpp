import ctypes
import os
import sys
from pathlib import Path

__all__ = ['context_default_params', 'init_context', 'process_full', 'free']

# Load both libraries
_lib_base_name_qwen2 = "nexa-qwen2-audio-lib"
_lib_base_name_omni = "nexa-omni-audio-lib"

base_path_qwen2 = (
    Path(__file__).parent.parent.parent.resolve()
    / "build"
    / "examples"
    / "qwen2-audio"
)

base_path_omni = (
    Path(__file__).parent.parent.parent.resolve()
    / "build"
    / "examples"
    / "nano-omni-audio"
)

def _load_shared_library(lib_base_name: str, base_path: Path = None):
    # Determine the file extension based on the platform
    if sys.platform.startswith("linux"):
        lib_ext = ".so"
    elif sys.platform == "darwin":
        lib_ext = ".dylib"
    elif sys.platform == "win32":
        lib_ext = ".dll"
    else:
        raise RuntimeError("Unsupported platform")

    # Construct the paths to the possible shared library names
    if base_path is None:
        _base_path = Path(__file__).parent.parent.resolve()
    else:
        _base_path = base_path
    _lib_paths = [
        _base_path / f"lib{lib_base_name}{lib_ext}",
        _base_path / f"{lib_base_name}{lib_ext}",
    ]

    # Add the library directory to the DLL search path on Windows (if needed)
    if sys.platform == "win32" and sys.version_info >= (3, 8):
        os.add_dll_directory(str(_base_path))

    # Try to load the shared library, handling potential errors
    for _lib_path in _lib_paths:
        if _lib_path.exists():
            try:
                return ctypes.CDLL(str(_lib_path))
            except Exception as e:
                print(f"Failed to load shared library '{_lib_path}': {e}")

    raise FileNotFoundError(
        f"Shared library with base name '{lib_base_name}' not found"
    )

# Load both libraries
_lib_qwen2 = _load_shared_library(_lib_base_name_qwen2, base_path_qwen2)
_lib_omni = _load_shared_library(_lib_base_name_omni, base_path_omni)

class omni_context_params(ctypes.Structure):
    _fields_ = [
        ("model", ctypes.c_char_p),
        ("mmproj", ctypes.c_char_p),
        ("file", ctypes.c_char_p),
        ("prompt", ctypes.c_char_p),
        ("n_gpu_layers", ctypes.c_int32),
    ]

omni_context_params_p = ctypes.POINTER(omni_context_params)

omni_context_p = ctypes.c_void_p

# Modified functions to use the appropriate library based on is_qwen parameter
def context_default_params(is_qwen: bool = False) -> omni_context_params:
    lib = _lib_qwen2 if is_qwen else _lib_omni
    return lib.omni_context_default_params()

def init_context(params: omni_context_params_p, is_qwen: bool = False) -> omni_context_p:  # type: ignore
    lib = _lib_qwen2 if is_qwen else _lib_omni
    return lib.omni_init_context(params)

def process_full(ctx: omni_context_p, params: omni_context_params_p, is_qwen: bool = False):  # type: ignore
    lib = _lib_qwen2 if is_qwen else _lib_omni
    return lib.omni_process_full(ctx, params)

def free(ctx: omni_context_p, is_qwen: bool = False):
    lib = _lib_qwen2 if is_qwen else _lib_omni
    return lib.omni_free(ctx)

# Set argtypes and restype for both libraries
for lib in [_lib_qwen2, _lib_omni]:
    lib.omni_context_default_params.argtypes = []
    lib.omni_context_default_params.restype = omni_context_params
    
    lib.omni_init_context.argtypes = [omni_context_params_p]
    lib.omni_init_context.restype = omni_context_p
    
    lib.omni_process_full.argtypes = [omni_context_p, omni_context_params_p]
    lib.omni_process_full.restype = None
    
    lib.omni_free.argtypes = [omni_context_p]
    lib.omni_free.restype = None
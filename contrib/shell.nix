with import <nixpkgs> {};
clangStdenv.mkDerivation {
  name = "clang-nix-shell";
  buildInputs = [ gcc check zlib meson ninja ];
}

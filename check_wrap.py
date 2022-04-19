import os

def check_if_directory_was_wrapped(dir_name: str) -> bool:
    wrap_files, input_files = 0, 0
    stack = [dir_name]
    while len(stack) != 0:
        active_dir = stack.pop()
        for f in os.listdir(active_dir):
            sub_dir_file_path = os.path.join(active_dir, f)
            if os.path.isdir(sub_dir_file_path):
                stack.append(sub_dir_file_path)
            else:
                if f.startswith("wrap."):
                    wrap_files += 1
                elif not f.startswith("."):
                    input_files += 1
        if wrap_files == input_files:
            #print(f"Good {active_dir}")
            pass
        else:
            print(f"Bad {active_dir} Wrap file: {wrap_files} and Input file: {input_files}")
        wrap_files, input_files = 0, 0
        


if __name__ == "__main__":
    check_if_directory_was_wrapped("big")
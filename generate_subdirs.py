"""
    Generate Sub-directories with text files filled with random words.

    Author: Rohan Deshpande.
"""
import os
import random
import string


class GenerateSubDir:
    def __init__(
        self, root, levels, files_range, subdirectory_range, paragraph_setting
    ) -> None:
        self.root = root
        self.levels = levels

        self.files_range = files_range
        self.subdirectory_range = subdirectory_range

        self.range_word_length = paragraph_setting["range_word_length"]
        self.range_sentence_length = paragraph_setting["range_sentence_length"]
        self.range_para_length = paragraph_setting["range_para_length"]

        self.alphabets = list(string.ascii_lowercase)

    def get_random_word(self):
        random_word_length = random.randint(
            self.range_word_length[0], self.range_word_length[1]
        )
        return "".join(random.sample(self.alphabets, random_word_length))

    def get_random_sentence(self):
        random_sentence_length = random.randint(
            self.range_sentence_length[0], self.range_sentence_length[1]
        )
        random_sentence = [
            self.get_random_word() for _ in range(random_sentence_length)
        ]
        return " ".join(random_sentence)

    def get_random_paragraph(self):
        random_paragraph_length = random.randint(
            self.range_para_length[0], self.range_para_length[1]
        )
        random_paragraph = [
            self.get_random_sentence() for _ in range(random_paragraph_length)
        ]
        return "\n".join(random_paragraph)

    def make_sub_dirs(self):
        if not os.path.exists(self.root):
            os.mkdir(self.root)
        stack = []
        level_count = 0
        root_dir_full_path = os.path.join(os.getcwd(), self.root)
        stack.append(root_dir_full_path)
        while len(stack) != 0:
            active_dir = stack.pop()

            random_number_of_dir = random.randint(
                self.subdirectory_range[0], self.subdirectory_range[1]
            )
            if level_count < self.levels:
                for _ in range(random_number_of_dir):
                    dir_name = self.get_random_word()
                    dir_path = os.path.join(active_dir, dir_name)
                    os.mkdir(dir_path)
                    stack.append(dir_path)

            random_number_of_files = random.randint(
                self.files_range[0], self.files_range[1]
            )
            for _ in range(random_number_of_files):
                file_name = self.get_random_word()
                file_path = os.path.join(active_dir, f"{file_name}.txt")
                para = self.get_random_paragraph()
                f = open(file_path, "a")
                f.write(para)
                f.close()
            level_count += 1


if __name__ == "__main__":
    gs = GenerateSubDir(
        root="d",
        levels=10,
        files_range=(1, 5),
        subdirectory_range=(1, 5),
        paragraph_setting={
            "range_word_length": (3, 6),
            "range_sentence_length": (5, 10),
            "range_para_length": (5, 10),
        },
    )
    gs.make_sub_dirs()

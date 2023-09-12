import os
import sys
import re



def print_list(list):
    for addr in list:
        os.system(f"addr2line -C -e build/foo_cc {addr}")



def parse_addresses_from_split_file_list(lines):
    addresses_in_line = []
    for line in lines:
        text_in_parens = re.findall(r'\(([^()]+)\)', line)
        if(text_in_parens[1] != "" and text_in_parens[1][0] == '+'):
            addresses_in_line += [text_in_parens[1][1:]]
    return addresses_in_line



def open_and_split_file(file_name):
    split_and_processed_file = []
    with open(file_name) as file:
        lines = file.readlines()
        for line in lines:
            stripped_line = line.replace("\n", "")
            if(stripped_line != "" and stripped_line[0] == '['):
                split_and_processed_file += [stripped_line]
    return split_and_processed_file


def main():
    if(len(sys.argv) != 2):
        print("Expected a file name parameter to program.")
        return 1
    split_and_processed_file = open_and_split_file(sys.argv[1])
    addresses_in_line = parse_addresses_from_split_file_list(split_and_processed_file)
    print_list(addresses_in_line)


if __name__ == "__main__":
    main()



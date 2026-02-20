# import pandas as pd
# import tkinter as tk
# from tkinter import filedialog, messagebox

# def bool_str(v):
#     try:
#         return "True" if int(v) == 1 else "False"
#     except:
#         return "False"

# def convert_xlsx_to_csv(input_path, output_path):
#     df = pd.read_excel(input_path)

#     rows = []

#     for _, row in df.iterrows():
#         item_id = str(row["ID"])
#         item_type = str(row["Type"]).upper()

#         text_data = (
#             f'(Name="{row["Name"]}",'
#             f'Description="{row["Description"]}")'
#         )

#         numeric_data = (
#             f'(IsStackable={bool_str(row["IsStackable"])},'
#             f'MaxAmount=0,'
#             f'Damage={float(row["Damage"]):.6f},'
#             f'Health={float(row["Health"]):.6f},'
#             f'Stamina={float(row["Stamina"]):.6f},'
#             f'Hydration={float(row["Water"]):.6f},'
#             f'Hunger={float(row["Hungry"]):.6f},'
#             f'Weight={float(row["Weight"]):.6f},'
#             f'Durability={float(row["Durability"]):.6f},'
#             f'InteractionDuration={float(row["InteractionDuration"]):.6f},'
#             f'IsUsable={bool_str(row["IsUsable"])},'
#             f'UseDuration={float(row["UseDuration"]):.6f})'
#         )

#         asset_data = "(Icon=None,Mesh=None,BPMesh=None)"

#         rows.append([
#             item_id,
#             item_id,
#             item_type,
#             text_data,
#             numeric_data,
#             asset_data,
#             "None"
#         ])

#     out_df = pd.DataFrame(
#         rows,
#         columns=["---", "ID", "Type", "TextData", "NumericData", "AssetData", "ItemClass"]
#     )

#     out_df.to_csv(output_path, index=False, encoding="utf-8-sig")


# # ================= GUI =================

# def select_input():
#     path = filedialog.askopenfilename(
#         title="입력 XLSX 파일 선택",
#         filetypes=[("Excel files", "*.xlsx")]
#     )
#     if path:
#         input_entry.delete(0, tk.END)
#         input_entry.insert(0, path)

# def select_output():
#     path = filedialog.asksaveasfilename(
#         title="출력 CSV 저장 위치",
#         defaultextension=".csv",
#         filetypes=[("CSV files", "*.csv")]
#     )
#     if path:
#         output_entry.delete(0, tk.END)
#         output_entry.insert(0, path)

# def run_convert():
#     input_path = input_entry.get()
#     output_path = output_entry.get()

#     if not input_path or not output_path:
#         messagebox.showerror("오류", "입력 파일과 출력 경로를 모두 선택하세요.")
#         return

#     try:
#         convert_xlsx_to_csv(input_path, output_path)
#         messagebox.showinfo("완료", "CSV 변환이 완료되었습니다.")
#     except Exception as e:
#         messagebox.showerror("변환 실패", str(e))


# root = tk.Tk()
# root.title("XLSX → UE DataTable CSV 변환기")
# root.geometry("600x200")
# root.resizable(False, False)

# tk.Label(root, text="입력 XLSX 파일").pack(pady=(15, 0))
# input_frame = tk.Frame(root)
# input_frame.pack(padx=10, fill="x")

# input_entry = tk.Entry(input_frame)
# input_entry.pack(side="left", fill="x", expand=True)
# tk.Button(input_frame, text="찾아보기", command=select_input).pack(side="right")

# tk.Label(root, text="출력 CSV 파일").pack(pady=(10, 0))
# output_frame = tk.Frame(root)
# output_frame.pack(padx=10, fill="x")

# output_entry = tk.Entry(output_frame)
# output_entry.pack(side="left", fill="x", expand=True)
# tk.Button(output_frame, text="저장 위치", command=select_output).pack(side="right")

# tk.Button(root, text="변환 실행", height=2, command=run_convert).pack(pady=20)

# root.mainloop()

import csv
import io
import tkinter as tk
from tkinter import filedialog, messagebox

def open_csv_any_encoding(path):
    for enc in ("utf-8-sig", "utf-16", "cp949"):
        try:
            with open(path, "r", encoding=enc, newline="") as f:
                return f.read()
        except UnicodeDecodeError:
            continue
    raise UnicodeDecodeError("encoding", b"", 0, 1, "지원하지 않는 인코딩")


def split_ue_single_column_csv(input_csv, output_csv):
    text = open_csv_any_encoding(input_csv)

    reader = csv.reader(io.StringIO(text))
    rows = list(reader)

    fixed_rows = []

    for row in rows:
        if not row:
            continue

        if len(row) > 1:
            fixed_rows.append(row)
            continue

        raw = row[0]

        try:
            parsed = next(csv.reader(io.StringIO(raw)))
            fixed_rows.append(parsed)
        except Exception:
            fixed_rows.append(row)

    with open(output_csv, "w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(fixed_rows)


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("UE CSV 컬럼 분리 도구")
        self.geometry("500x380")
        self.resizable(False, False)

        self.input_path = tk.StringVar()
        self.output_path = tk.StringVar()

        self.create_widgets()

    def create_widgets(self):
        tk.Label(self, text="입력 CSV (UE Export)").pack(pady=5)
        tk.Entry(self, textvariable=self.input_path, width=60).pack()
        tk.Button(self, text="입력 파일 선택", command=self.select_input).pack(pady=5)

        tk.Label(self, text="출력 CSV (분리된 파일)").pack(pady=5)
        tk.Entry(self, textvariable=self.output_path, width=60).pack()
        tk.Button(self, text="출력 위치 선택", command=self.select_output).pack(pady=5)

        tk.Button(self, text="변환 실행", command=self.run, height=2).pack(pady=10)

    def select_input(self):
        path = filedialog.askopenfilename(
            filetypes=[("CSV Files", "*.csv")]
        )
        if path:
            self.input_path.set(path)

    def select_output(self):
        path = filedialog.asksaveasfilename(
            defaultextension=".csv",
            filetypes=[("CSV Files", "*.csv")]
        )
        if path:
            self.output_path.set(path)

    def run(self):
        if not self.input_path.get() or not self.output_path.get():
            messagebox.showerror("오류", "입력 파일과 출력 위치를 모두 선택하세요.")
            return

        try:
            split_ue_single_column_csv(
                self.input_path.get(),
                self.output_path.get()
            )
            messagebox.showinfo("완료", "CSV 컬럼 분리가 완료되었습니다.")
        except Exception as e:
            messagebox.showerror("실패", str(e))


if __name__ == "__main__":
    App().mainloop()

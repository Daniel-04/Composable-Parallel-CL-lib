# manim -pqk animations.py

from manim import *
from manim.utils.color.manim_colors import _all_manim_colors
from math import *
import random


class AnimatedMap(Scene):
    def construct(self):
        self.init_config()
        self.create_global_matrices()
        self.create_shared_memory()
        self.create_threads()
        self.play_setup_animations()
        self.connect_global_to_shared()
        self.wait(1)
        self.perform_addition_global()
        self.wait(2)
        self.create_shared_memory()
        self.connect_global_to_shared()
        self.display_memory_sections()
        self.wait(1)
        self.perform_addition_shared()
        self.write_back_to_global()
        self.cleanup()
        self.wait(3)
        self.perform_addition_global()
        self.wait(1)

    def init_config(self):
        self.group_size = 4
        self.array_size = 16
        self.work_size = self.array_size
        self.scale_factor = min(1.0, 10 / self.array_size)
        self.data_A = list(range(1, self.array_size + 1))
        self.data_B = list(map(lambda x: ceil(sqrt(93 * x)), self.data_A))
        self.colours = _all_manim_colors
        random.seed(93)
        random.shuffle(self.colours)

    def display_memory_sections(self):
        split_line = Line(LEFT * config.frame_width / 2, RIGHT * config.frame_width / 2)
        self.play(Create(split_line))
        self.play(Write(Text("Global\nMemory").align_to(split_line, DOWN + RIGHT)))
        self.play(Write(Text("Shared\nMemory").align_to(split_line, UP + RIGHT)))

    def create_global_matrices(self):
        self.A = Matrix([self.data_A]).scale(self.scale_factor).to_edge(UP + LEFT)
        self.A_groups = [
            self.A.get_entries()[i : min(i + self.group_size, self.work_size)]
            for i in range(0, self.work_size, self.group_size)
        ]

        self.B = Matrix([self.data_B]).scale(self.scale_factor).next_to(self.A, DOWN)
        self.B_groups = [
            self.B.get_entries()[i : min(i + self.group_size, self.work_size)]
            for i in range(0, self.work_size, self.group_size)
        ]

    def create_shared_memory(self):
        self.shares = [
            [
                self.data_A[i : min(i + self.group_size, self.work_size)],
                self.data_B[i : min(i + self.group_size, self.work_size)],
            ]
            for i in range(0, self.work_size, self.group_size)
        ]
        if self.work_size % self.group_size != 0:
            padding = ["?"] * (self.group_size - (self.work_size % self.group_size))
            self.shares[-1].extend(padding)

        self.SHARES = [Matrix(share).scale(self.scale_factor) for share in self.shares]
        self.SHARES[0].to_edge(DOWN + LEFT)
        for i in range(1, len(self.SHARES)):
            self.SHARES[i].next_to(self.SHARES[i - 1], RIGHT)

    def connect_global_to_shared(self):
        self.ARROWS = VGroup()
        self.UNDERLINES = VGroup()
        for idx, elems in enumerate(self.B_groups):
            underline = Underline(elems, color=self.colours[idx])
            self.UNDERLINES.add(underline)

            arrow = Arrow(
                underline, self.SHARES[idx].get_top(), color=self.colours[idx]
            )
            self.ARROWS.add(arrow)
            self.SHARES[idx].set_color(self.colours[idx])

    def create_threads(self):
        thread_count = self.group_size * len(self.shares)
        self.THREADS = VGroup(
            *[Text(f"t{t}", font_size=16, color=GOLD) for t in range(thread_count)]
        )
        for idx, thread in enumerate(self.THREADS):
            group_idx = idx // self.group_size
            local_idx = idx % self.group_size
            entry = self.SHARES[group_idx].get_entries()[local_idx]
            thread.next_to(entry, DOWN, buff=0.8)
        self.add(self.THREADS)

    def play_setup_animations(self):
        self.play(Create(self.A))
        self.play(Create(self.B))

    def perform_addition_global(self):
        pluses = [Text("+").next_to(d, DOWN) for d in self.B.get_entries()]

        active_threads = [Indicate(t) for t in self.THREADS]
        added_elements = []
        update_animations = []

        for idx, val in enumerate(self.data_A):
            added_elements.append(
                self.A.get_entries()[idx]
                .copy()
                .animate.move_to(self.B.get_entries()[idx])
                .fade(1)
            )

            self.data_B[idx] += self.data_A[idx]

        new_matrix = self.copy_matrix_with_new_values(self.B, [self.data_B])
        update_animations.extend([FadeIn(new_matrix), FadeOut(self.B)])
        self.B = new_matrix

        self.play(*[Indicate(p) for p in pluses])
        self.play(*active_threads, *[FadeOut(p) for p in pluses])
        self.play(*added_elements, *update_animations)

    def perform_addition_shared(self):
        self.play(
            Create(self.UNDERLINES),
            Create(self.ARROWS),
            *[Create(s) for s in self.SHARES],
        )
        self.play(FadeOut(self.ARROWS))

        pluses = [Text("+").next_to(s, UP + LEFT) for s in self.SHARES]
        self.play(*[Indicate(p) for p in pluses])
        self.play(*[FadeOut(p) for p in pluses])

        active_threads = [Indicate(t) for t in self.THREADS]
        added_elements = []
        update_animations = []

        for idx, share_group in enumerate(self.shares):
            old_entries = self.SHARES[idx].get_entries()
            for j in range(self.group_size):
                source_idx = j + self.work_size
                target_idx = j
                if source_idx < len(old_entries):
                    source_elem = old_entries[source_idx]
                    target_elem = old_entries[target_idx]
                    added_elements.append(
                        source_elem.copy().animate.move_to(target_elem).fade(1)
                    )

            for j in range(len(share_group[1])):
                try:
                    share_group[1][j] += share_group[0][j]
                    self.data_B[j + idx * self.group_size] = share_group[1][j]
                except TypeError:
                    pass

            new_matrix = self.copy_matrix_with_new_values(self.SHARES[idx], share_group)
            update_animations.extend([FadeIn(new_matrix), FadeOut(self.SHARES[idx])])
            self.SHARES[idx] = new_matrix

        self.play(*active_threads)
        self.play(*added_elements, *update_animations)

    def write_back_to_global(self):
        back_arrows = VGroup()
        new_vals_mo = []
        old_vals_mo = self.B.get_entries()

        for idx, share_matrix in enumerate(self.SHARES):
            for j in range(self.group_size):
                source = share_matrix.get_entries()[self.group_size + j]
                target = self.B.get_entries()[j + idx * self.group_size]
                back_arrows.add(Arrow(source, target, color=self.colours[idx]))

                new_val = source.copy()
                new_vals_mo.append(new_val)
                self.B.add(new_val)

        self.play(Create(back_arrows))
        self.play(
            *[
                val.animate.move_to(old.fade(1))
                for val, old in zip(new_vals_mo, old_vals_mo)
            ]
        )
        self.play(FadeOut(back_arrows))

    def cleanup(self):
        self.play(*[FadeOut(s) for s in self.SHARES], FadeOut(self.UNDERLINES))

    def copy_matrix_with_new_values(self, mat, new_values):
        new_mat = Matrix(new_values)
        new_mat.set_color(mat.get_color())
        new_mat.scale(self.scale_factor)
        new_mat.move_to(mat.get_center())
        return new_mat


class AnimatedOuterProduct(Scene):
    def construct(self):
        self.init_config()
        self.display_memory_sections()
        self.create_global_matrices()
        self.play_setup_animations()
        self.show_reused_items()
        self.create_shared_memory()
        self.connect_global_to_shared()
        self.create_threads()
        for row in range(len(self.data_A)):
            self.perform_addition_shared(row)
        self.wait(1)

    def init_config(self):
        self.group_size = 4
        self.data_A = [[n] for n in range(1, 4)]
        self.data_B = list(range(4, 11))
        self.scale_factor = min(
            1.0, 10 / max(len(self.data_A) * 10 / 4, len(self.data_B))
        )
        self.data_C = [
            ["?" for _ in range(len(self.data_B))] for _ in range(len(self.data_A))
        ]
        self.colours = _all_manim_colors
        random.seed(93)
        random.shuffle(self.colours)

    def display_memory_sections(self):
        split_line = Line(LEFT * config.frame_width / 2, RIGHT * config.frame_width / 2)
        self.add(split_line)
        self.add(Text("Global\nMemory").align_to(split_line, DOWN + RIGHT))
        self.add(Text("Shared\nMemory").align_to(split_line, UP + RIGHT))

    def create_global_matrices(self):
        self.B = Matrix([self.data_B]).scale(self.scale_factor)
        self.B_groups = [
            self.B.get_entries()[i : min(i + self.group_size, len(self.data_B))]
            for i in range(0, len(self.data_B), self.group_size)
        ]

        self.C = Matrix(self.data_C).scale(self.scale_factor).next_to(self.B, DOWN)

        self.A = Matrix(self.data_A).scale(self.scale_factor).next_to(self.C, LEFT)
        self.A_groups = [
            self.A.get_entries()[i : min(i + self.group_size, len(self.data_A))]
            for i in range(0, len(self.data_A), self.group_size)
        ]
        VGroup(self.A, self.B, self.C).to_edge(LEFT + UP, buff=0.1)

    def create_shared_memory(self):
        self.shares = [
            [
                [i[0] for i in self.data_A],
                self.data_B[i : min(i + self.group_size, len(self.data_B))],
            ]
            for i in range(0, len(self.data_B), self.group_size)
        ]
        for share in self.shares:
            for row in share:
                if len(row) < self.group_size:
                    padding = ["?"] * (self.group_size - (len(row) % self.group_size))
                    row.extend(padding)

        SHARE_GROUP = VGroup()
        self.SHARES = [Matrix(share).scale(self.scale_factor) for share in self.shares]
        SHARE_GROUP.add(self.SHARES[0])
        for i in range(1, len(self.SHARES)):
            self.SHARES[i].next_to(self.SHARES[i - 1], RIGHT)
            SHARE_GROUP.add(self.SHARES[i])

        SHARE_GROUP.next_to(self.C, DOWN).to_edge(DOWN)

    def connect_global_to_shared(self):
        self.ARROWS = VGroup()
        for idx, elems in enumerate(self.B_groups):
            arrow = Arrow(
                self.B.get_center(),
                self.SHARES[idx].get_center(),
                color=self.colours[idx],
            )
            self.ARROWS.add(arrow)
            arrow_a = Arrow(
                self.A.get_bottom(),
                self.SHARES[idx].get_corner(UP + LEFT),
                color=self.colours[idx],
            )
            self.ARROWS.add(arrow_a)
            self.SHARES[idx].set_color(self.colours[idx])

        self.play(
            Create(self.ARROWS),
            *[Create(s) for s in self.SHARES],
        )
        self.play(FadeOut(self.ARROWS))

    def create_threads(self):
        thread_count = self.group_size * len(self.SHARES)
        self.THREADS = VGroup(
            *[Text(f"t{t}", font_size=16, color=GOLD) for t in range(thread_count)]
        )
        for idx, thread in enumerate(self.THREADS):
            group_idx = idx // self.group_size
            local_idx = idx % self.group_size
            entry = self.SHARES[group_idx].get_entries()[local_idx]
            thread.next_to(entry, DOWN, buff=1)
        self.add(self.THREADS)

    def play_setup_animations(self):
        self.play(Create(self.A), Create(self.B), Create(self.C))
        self.play(Indicate(Text("+").next_to(self.A, UP)))

    def show_reused_items(self):
        original_entries = [entry.copy() for entry in self.C.get_entries()]

        for i in range(len(self.data_B)):
            entry = self.C.get_entries()[i]
            new_entry = MathTex(f"{self.data_A[0][0] + self.data_B[i]}").move_to(entry)
            self.play(
                Indicate(self.A.get_entries()[0]),
                Indicate(self.B.get_entries()[i]),
                Circumscribe(entry),
                Transform(entry, new_entry),
            )

        for i in range(len(self.data_A)):
            entry = self.C.get_entries()[i * len(self.data_B)]
            new_entry = MathTex(f"{self.data_A[i][0] + self.data_B[0]}").move_to(entry)
            self.play(
                Indicate(self.A.get_entries()[i]),
                Indicate(self.B.get_entries()[0]),
                Circumscribe(entry),
                Transform(entry, new_entry),
            )

        current_entries = self.C.get_entries()
        reverts = [
            Transform(current_entries[i], original_entries[i])
            for i in range(len(current_entries))
        ]
        self.play(*reverts)

    def perform_addition_shared(self, row):
        pluses = [Text("+").next_to(s, UP + LEFT) for s in self.SHARES]

        active_threads = [Indicate(t) for t in self.THREADS]
        added_elements = []
        update_animations = []

        for idx, share_group in enumerate(self.shares):
            old_entries = self.SHARES[idx].get_entries()
            for j in range(len(share_group[1])):
                target_idx = j + self.group_size * idx + row * len(self.data_B)
                if target_idx < len(self.C.get_entries()):
                    source_elem_1 = old_entries[row]
                    source_elem_2 = old_entries[j + self.group_size]
                    target_elem = self.C.get_entries()[target_idx]
                    added_elements.append(
                        source_elem_1.copy().animate.move_to(target_elem).fade(1)
                    )
                    added_elements.append(
                        source_elem_2.copy().animate.move_to(target_elem).fade(1)
                    )
                    try:
                        self.data_C[row][j + self.group_size * idx] = (
                            share_group[1][j] + share_group[0][row]
                        )
                    except TypeError:
                        pass

            new_matrix = self.copy_matrix_with_new_values(self.C, self.data_C)
            update_animations.extend([FadeIn(new_matrix), FadeOut(self.C)])
            self.C = new_matrix

        self.play(
            *active_threads,
            *added_elements,
            *update_animations,
            *[Indicate(p) for p in pluses],
            *[FadeOut(p) for p in pluses],
        )

    def copy_matrix_with_new_values(self, mat, new_values):
        new_mat = Matrix(new_values)
        new_mat.set_color(mat.get_color())
        new_mat.scale(self.scale_factor)
        new_mat.move_to(mat.get_center())
        return new_mat


class AnimatedReduce(Scene):
    def construct(self):
        self.init_config()
        self.display_memory_sections()

        for _ in range(self.num_iterations):
            self.create_global_matrix()
            self.create_shared_memory()
            self.connect_global_to_shared()
            self.create_threads()
            self.play_setup_animations()
            self.perform_reduction_phase()
            self.write_back_to_global()
            self.cleanup_after_iteration()

    def init_config(self):
        self.group_size = 4
        self.array_size = 16
        self.work_size = self.array_size
        self.scale_factor = min(1.0, 10 / self.array_size)
        self.data = list(range(1, self.array_size + 1))
        self.num_iterations = ceil(log(self.array_size, self.group_size))
        self.colours = _all_manim_colors
        random.seed(93)
        random.shuffle(self.colours)

    def display_memory_sections(self):
        split_line = Line(LEFT * config.frame_width / 2, RIGHT * config.frame_width / 2)
        self.add(split_line)
        self.add(Text("Global\nMemory").align_to(split_line, DOWN + RIGHT))
        self.add(Text("Shared\nMemory").align_to(split_line, UP + RIGHT))

    def create_global_matrix(self):
        self.A = Matrix([self.data]).scale(self.scale_factor).to_edge(UP + LEFT)
        self.A_groups = [
            self.A.get_entries()[i : min(i + self.group_size, self.work_size)]
            for i in range(0, self.work_size, self.group_size)
        ]

    def create_shared_memory(self):
        self.shares = [
            self.data[i : min(i + self.group_size, self.work_size)]
            for i in range(0, self.work_size, self.group_size)
        ]
        if self.work_size % self.group_size != 0:
            padding = ["?"] * (self.group_size - (self.work_size % self.group_size))
            self.shares[-1].extend(padding)

        self.SHARES = [
            Matrix([share]).scale(self.scale_factor) for share in self.shares
        ]
        self.SHARES[0].to_edge(DOWN + LEFT)
        for i in range(1, len(self.SHARES)):
            self.SHARES[i].next_to(self.SHARES[i - 1], RIGHT)

    def connect_global_to_shared(self):
        self.ARROWS = VGroup()
        for idx, elems in enumerate(self.A_groups):
            underline = Underline(elems, color=self.colours[idx])
            self.A.add(underline)

            arrow = Arrow(
                underline, self.SHARES[idx].get_top(), color=self.colours[idx]
            )
            self.ARROWS.add(arrow)
            self.SHARES[idx].set_color(self.colours[idx])

    def create_threads(self):
        thread_count = self.group_size * len(self.shares)
        self.THREADS = VGroup(
            *[Text(f"t{t}", font_size=16, color=GOLD) for t in range(thread_count)]
        )
        for idx, thread in enumerate(self.THREADS):
            group_idx = idx // self.group_size
            local_idx = idx % self.group_size
            entry = self.SHARES[group_idx].get_entries()[local_idx]
            thread.next_to(entry, DOWN)
        self.add(self.THREADS)

    def play_setup_animations(self):
        self.play(Create(self.A))
        self.play(Create(self.ARROWS), *[Create(s) for s in self.SHARES])
        self.play(FadeOut(self.ARROWS))

    def perform_reduction_phase(self):
        pluses = [Text("+").next_to(s, UP + LEFT) for s in self.SHARES]
        self.play(*[Indicate(p) for p in pluses])
        self.play(*[FadeOut(p) for p in pluses], run_time=0.2)

        stride = self.group_size // 2
        while stride > 0:
            self.reduction_step(stride)
            stride //= 2

    def reduction_step(self, stride):
        active_threads = []
        added_elements = []
        update_animations = []

        for idx, share_group in enumerate(self.shares):
            start = self.group_size * idx
            end = start + stride
            active_threads.extend(Indicate(t) for t in self.THREADS[start:end])

            old_entries = self.SHARES[idx].get_entries()
            for j in range(stride):
                source_idx = j + stride
                target_idx = j
                if source_idx < len(old_entries):
                    source_elem = old_entries[source_idx]
                    target_elem = old_entries[target_idx]
                    added_elements.append(
                        source_elem.copy().animate.move_to(target_elem).fade(1)
                    )

            for j in range(len(share_group) // 2):
                try:
                    share_group[j] += share_group[j + stride]
                except TypeError:
                    pass

            new_matrix = self.copy_matrix_with_new_values(
                self.SHARES[idx], [share_group]
            )
            update_animations.extend([FadeIn(new_matrix), FadeOut(self.SHARES[idx])])
            self.SHARES[idx] = new_matrix
            self.data[idx] = share_group[0]

        self.play(*active_threads)
        self.play(*added_elements, *update_animations)

    def write_back_to_global(self):
        back_arrows = VGroup()
        new_vals_mo = []
        old_vals_mo = self.A.get_entries()[: len(self.SHARES)]

        for idx, share_matrix in enumerate(self.SHARES):
            source = share_matrix.get_entries()[0]
            target = self.A.get_entries()[idx]
            back_arrows.add(Arrow(source, target, color=self.colours[idx]))

            new_val = source.copy()
            new_vals_mo.append(new_val)
            self.A.add(new_val)

        self.play(Create(back_arrows))
        self.play(
            *[
                val.animate.move_to(old.fade(1))
                for val, old in zip(new_vals_mo, old_vals_mo)
            ]
        )
        self.play(FadeOut(back_arrows))

    def cleanup_after_iteration(self):
        self.play(
            FadeOut(self.A), FadeOut(self.THREADS), *[FadeOut(s) for s in self.SHARES]
        )

        self.work_size = (self.work_size + self.group_size - 1) // self.group_size

    def copy_matrix_with_new_values(self, mat, new_values):
        new_mat = Matrix(new_values)
        new_mat.set_color(mat.get_color())
        new_mat.scale(self.scale_factor)
        new_mat.move_to(mat.get_center())
        return new_mat


class AnimatedScan(Scene):
    def construct(self):
        self.init_config()
        self.display_memory_sections()
        self.create_global_matrix()
        self.create_shared_memory()
        self.connect_global_to_shared()
        self.play_setup_animations()
        self.perform_scan_phase()
        self.write_back_to_global()
        self.perform_post_processing()

    def init_config(self):
        self.group_size = 4
        self.array_size = 16
        self.work_size = self.array_size
        self.scale_factor = min(1.0, 10 / self.array_size)

        self.colours = _all_manim_colors
        random.seed(93)
        random.shuffle(self.colours)

    def display_memory_sections(self):
        split_line = Line(LEFT * config.frame_width / 2, RIGHT * config.frame_width / 2)
        self.add(split_line)
        self.add(Text("Global\nMemory").align_to(split_line, DOWN + RIGHT))
        self.add(Text("Shared\nMemory").align_to(split_line, UP + RIGHT))

    def create_global_matrix(self):
        self.data = list(range(1, self.array_size + 1))

        self.A = Matrix([self.data])
        self.A.scale(self.scale_factor)
        self.A.to_edge(UP + LEFT)
        self.add(self.A)

        self.A_groups = [
            self.A.get_entries()[i : min(i + self.group_size, self.work_size)]
            for i in range(0, self.work_size, self.group_size)
        ]

    def create_shared_memory(self):
        self.shares = [
            self.data[i : min(i + self.group_size, self.work_size)]
            for i in range(0, self.work_size, self.group_size)
        ]
        if self.work_size % self.group_size != 0:
            self.shares[-1].extend(
                [0] * (self.group_size - (self.work_size % self.group_size))
            )

        self.SHARES = [Matrix([share]) for share in self.shares]

        for mat in self.SHARES:
            mat.scale(self.scale_factor)

        self.SHARES[0].to_edge(DOWN + LEFT)
        for i in range(1, len(self.SHARES)):
            self.SHARES[i].next_to(self.SHARES[i - 1], RIGHT)

    def connect_global_to_shared(self):
        self.ARROWS = VGroup()
        for idx, elems in enumerate(self.A_groups):
            underline = Underline(elems, color=self.colours[idx])
            self.A.add(underline)

            arrow = Arrow(
                underline, self.SHARES[idx].get_top(), color=self.colours[idx]
            )
            self.ARROWS.add(arrow)
            self.SHARES[idx].set_color(self.colours[idx])

        self.create_threads()

    def create_threads(self):
        threads = range(self.group_size * len(self.shares))
        self.THREADS = [Text(f"t{t}", font_size=16, color=GOLD) for t in threads]
        for idx, thread in enumerate(self.THREADS):
            matrix = self.SHARES[idx // self.group_size]
            thread.next_to(matrix.get_entries()[idx % self.group_size], DOWN)
        self.add(*self.THREADS)

    def play_setup_animations(self):
        self.play(Create(self.A))
        self.play(Create(self.ARROWS), *map(Create, self.SHARES))
        self.wait(0.5)
        self.play(FadeOut(self.ARROWS))

    def perform_scan_phase(self):
        offset = 1
        while offset < self.group_size:
            self.scan_step(offset)
            offset *= 2

    def scan_step(self, offset):
        active_threads = []
        added_elements = []
        update_animations = []

        for idx, share in enumerate(self.shares):
            thread_slice = self.THREADS[
                self.group_size * idx + offset : self.group_size * (idx + 1)
            ]
            active_threads.extend(map(Indicate, thread_slice))

            new_vals = []
            old_entries = self.SHARES[idx].get_entries()
            for j in range(len(share)):
                if j >= offset:
                    new_val = share[j] + share[j - offset]
                    new_vals.append(new_val)
                    added_elements.append(
                        old_entries[j - offset]
                        .copy()
                        .animate.move_to(old_entries[j])
                        .fade(1)
                    )
                else:
                    new_vals.append(share[j])

            self.shares[idx] = new_vals
            new_matrix = self.copy_matrix_with_values(self.SHARES[idx], [new_vals])
            update_animations += [FadeIn(new_matrix), FadeOut(self.SHARES[idx])]
            self.SHARES[idx] = new_matrix
            self.data[idx] = new_vals[0]

        self.play(*active_threads)
        self.play(*added_elements, *update_animations)

    def write_back_to_global(self):
        new_vals = []
        back_arrows = VGroup()

        for idx, share in enumerate(self.SHARES):
            for i, val in enumerate(share.get_entries()):
                global_idx = self.group_size * idx + i
                if global_idx < self.array_size:
                    target = self.A.get_entries()[global_idx]
                    arrow = Arrow(val, target, color=self.colours[idx])
                    back_arrows.add(arrow)
                    new_vals.append(val.copy())
                    self.data[global_idx] = self.shares[idx][i]

        for val in new_vals:
            self.A.add(val)

        old_entries = self.A.get_entries()
        self.play(Create(back_arrows))
        self.play(
            *(
                val.animate.move_to(old.fade(1))
                for val, old in zip(new_vals, old_entries)
            )
        )
        self.wait(0.5)
        self.play(FadeOut(back_arrows), *map(FadeOut, self.SHARES))

        self.replace_matrix_with_values(self.data)

    def replace_matrix_with_values(self, new_data):
        old_matrix = self.A
        self.A = Matrix([new_data]).scale(self.scale_factor).to_edge(UP + LEFT)
        self.play(FadeOut(old_matrix), FadeIn(self.A))

    def perform_post_processing(self):
        group_size = self.group_size
        while group_size < self.array_size:
            self.post_processing_step(group_size)
            self.wait(1)
            group_size *= 2

    def post_processing_step(self, group_size):
        groups = [
            self.A.get_entries()[i : min(i + group_size, self.array_size)]
            for i in range(0, self.array_size, group_size)
        ]
        partials = [
            self.data[i] for i in range(group_size - 1, self.array_size, group_size)
        ]
        PARTIALS = [group[-1].copy() for group in groups]
        arrows = VGroup()
        active_threads = []
        added_elements = []
        update_anims = []

        for idx, elems in enumerate(groups):
            group_start = group_size * idx
            group_end = group_start + group_size
            if idx % 2 == 0 and group_end < len(self.THREADS):
                underline = Underline(elems, color=self.colours[idx])
                self.A.add(underline)

                p_copy = PARTIALS[idx]
                p_copy.next_to(self.THREADS[group_end], UP)
                arrows.add(Arrow(underline, p_copy.get_top(), color=self.colours[idx]))
                p_copy.set_color(self.colours[idx])

                for i in range(group_size):
                    if group_end + i < self.array_size:
                        self.data[group_end + i] += partials[idx]
            else:
                active_threads.extend(
                    map(Indicate, self.THREADS[group_start:group_end])
                )
                added_elements.extend(
                    p_copy.copy().animate.move_to(entry).fade(1) for entry in elems
                )

        old_matrix = self.A
        self.A = self.copy_matrix_with_values(self.A, [self.data])
        update_anims += [FadeOut(old_matrix), FadeIn(self.A)]

        self.play(Create(arrows), *map(Create, PARTIALS[::2]))
        self.play(FadeOut(arrows))
        self.play(*active_threads)
        self.play(*added_elements, *update_anims)
        self.play(*map(FadeOut, PARTIALS[::2]))

    def copy_matrix_with_values(self, mat, new_values):
        new_mat = Matrix(new_values)
        new_mat.set_color(mat.get_color())
        new_mat.scale(self.scale_factor)
        new_mat.move_to(mat.get_center())
        return new_mat


class AnimatedTranspose(Scene):
    def construct(self):
        self.init_config()
        self.display_memory_sections()
        self.create_global_matrices()
        self.create_shared_memory()
        self.create_threads()
        self.play_setup_animations()
        for i in range(ceil(self.array_size / self.group_size) ** 2):
            self.connect_global_to_shared(i)
            self.write_back_transposed(i)
            self.cleanup(i)
        self.reset_B()
        for i in range(ceil(self.array_size / self.group_size) ** 2):
            self.connect_global_to_shared(i)
            self.write_back_transposed_show_banks(i)
            self.cleanup(i)
        self.reset_B()
        self.create_shared_memory(pad=True)
        for i in range(ceil(self.array_size / self.group_size) ** 2):
            self.connect_global_to_shared(i)
            self.write_back_transposed_padded(i)
            self.cleanup(i)
        self.wait(1)

    def init_config(self):
        self.group_size = 4
        self.array_size = 5
        self.work_size = self.array_size
        self.scale_factor = min(1.0, 3 / self.array_size)

        self.colours = _all_manim_colors
        random.seed(93)
        random.shuffle(self.colours)

    def display_memory_sections(self):
        split_line = Line(LEFT * config.frame_width / 2, RIGHT * config.frame_width / 2)
        split_line.shift(UP)
        self.add(split_line)
        self.add(Text("Global\nMemory").align_to(split_line, DOWN + RIGHT))
        self.add(Text("Shared\nMemory").align_to(split_line, UP + RIGHT))

    def create_global_matrices(self):
        self.data = [
            [i for i in range(j, j + self.array_size)]
            for j in range(0, self.array_size * self.array_size, self.array_size)
        ]
        self.unknown_data = [["?" for col in row] for row in self.data]

        self.A = Matrix(self.data)
        self.A.scale(self.scale_factor)
        self.A.to_edge(UP + LEFT)
        self.B = self.copy_matrix_with_values(self.A, self.unknown_data).next_to(
            self.A, RIGHT, buff=2
        )

        self.A_groups = [
            Group() for tile in range(ceil(self.array_size / self.group_size) ** 2)
        ]
        for i in range(self.array_size):
            for j in range(self.array_size):
                self.A_groups[
                    (i // self.group_size)
                    + (j // self.group_size) * ceil(self.array_size / self.group_size)
                ].add(self.A.get_entries()[i + j * self.array_size])

    def create_shared_memory(self, pad=False):
        self.shares = [
            [row[i : i + self.group_size] for row in self.data[j : j + self.group_size]]
            for j in range(0, self.work_size, self.group_size)
            for i in range(0, len(self.data[0]), self.group_size)
        ]

        for share in self.shares:
            for row in share:
                if len(row) < self.group_size:
                    row.extend(
                        [0] * (self.group_size - (self.work_size % self.group_size))
                    )

            if len(share) < self.group_size:
                share.extend(
                    [
                        [0] * self.group_size
                        for _ in range(
                            self.group_size - (self.work_size % self.group_size)
                        )
                    ]
                )

        if pad:
            for idx, share in enumerate(self.shares):
                new_share = []
                flat = [elem for row in share for elem in row]
                for _ in range(self.group_size + 1):
                    new_share.append([])
                    for i in range(self.group_size - 1):
                        new_share[-1].append(flat.pop(0))
                    new_share[-1].append(0)
                new_share.append([])
                new_share[-1].extend([flat.pop(0)] + [0] * (self.group_size - 1))
                self.shares[idx] = new_share

        self.SHARES = [
            IntegerTable(share, include_outer_lines=True) for share in self.shares
        ]

        for mat in self.SHARES:
            mat.scale(self.scale_factor)

        for SHARE in self.SHARES:
            SHARE.to_edge(DOWN)

    def create_threads(self):
        threads = list(range(self.group_size))
        self.THREADS = [
            Text(f"t{t}", font_size=16, color=GOLD)
            .next_to(self.A.get_entries()[t], DOWN)
            .to_edge(DOWN)
            for t in threads
        ]

        self.add(*self.THREADS)

    def play_setup_animations(self):
        self.play(Create(self.A))
        self.play(Create(self.B))

    def connect_global_to_shared(self, idx):
        group = self.A_groups[idx]
        share = self.SHARES[idx]
        arrow = Arrow(group, share)
        self.play(Circumscribe(group), FadeOut(arrow), Create(share))

    def write_back_transposed(self, idx):
        share = self.SHARES[idx]
        old_entries = share.get_entries()
        active_threads = [Indicate(T) for T in self.THREADS]

        tiles_per_row = ceil(self.array_size / self.group_size)
        tile_col = idx // tiles_per_row
        tile_row = idx % tiles_per_row

        for row in range(self.group_size):

            moves = []
            for col in range(self.group_size):
                global_col = (tile_col * self.group_size) + col
                global_row = (tile_row * self.group_size) + row
                target_idx = global_col + global_row * self.array_size
                source_idx = col * self.group_size + row
                if global_col < self.array_size and global_row < self.array_size:
                    entry = old_entries[source_idx].copy()
                    moves.append(
                        entry.animate.move_to(self.B.get_entries()[target_idx].fade(1))
                    )
                    self.B.add(entry)

            if moves:
                self.play(*active_threads, *moves)

    def write_back_transposed_show_banks(self, idx):
        share = self.SHARES[idx]
        old_entries = share.get_entries()
        banks = [
            Text(f"b{b}", font_size=16, color=TEAL).next_to(
                old_entries[len(old_entries) - self.group_size + b], DOWN
            )
            for b in range(self.group_size)
        ]
        self.play(*[Create(b) for b in banks])
        active_threads = [Indicate(T) for T in self.THREADS]

        tiles_per_row = ceil(self.array_size / self.group_size)
        tile_col = idx // tiles_per_row
        tile_row = idx % tiles_per_row

        for row in range(self.group_size):

            moves = []
            for col in range(self.group_size):
                global_col = (tile_col * self.group_size) + col
                global_row = (tile_row * self.group_size) + row
                target_idx = global_col + global_row * self.array_size
                source_idx = col * self.group_size + row
                if global_col < self.array_size and global_row < self.array_size:
                    entry = old_entries[source_idx].copy()
                    moves.append(
                        entry.animate.move_to(self.B.get_entries()[target_idx].fade(1))
                    )
                    self.B.add(entry)

                if moves:
                    self.play(*active_threads, *moves, Indicate(banks[row]))

        self.play(*[FadeOut(b) for b in banks])

    def write_back_transposed_padded(self, idx):
        share = self.SHARES[idx]
        old_entries = share.get_entries()
        active_threads = [Indicate(T) for T in self.THREADS]

        tiles_per_row = ceil(self.array_size / self.group_size)
        tile_col = idx // tiles_per_row
        tile_row = idx % tiles_per_row

        for row in range(self.group_size):

            moves = []
            for col in range(self.group_size):
                global_col = (tile_col * self.group_size) + col
                global_row = (tile_row * self.group_size) + row
                target_idx = global_col + global_row * self.array_size
                source_idx = col * self.group_size + row
                source_idx = source_idx + (source_idx // (self.group_size - 1))
                if global_col < self.array_size and global_row < self.array_size:
                    entry = old_entries[source_idx].copy()
                    moves.append(
                        entry.animate.move_to(self.B.get_entries()[target_idx].fade(1))
                    )
                    self.B.add(entry)

            if moves:
                self.play(*active_threads, *moves)

    def cleanup(self, idx):
        share = self.SHARES[idx]
        self.play(FadeOut(share))

    def reset_B(self):
        old = self.B
        self.B = self.copy_matrix_with_values(self.B, self.unknown_data)
        self.play(FadeOut(old), FadeIn(self.B))

    def copy_matrix_with_values(self, mat, new_values):
        new_mat = Matrix(new_values)
        new_mat.set_color(mat.get_color())
        new_mat.scale(self.scale_factor)
        new_mat.move_to(mat.get_center())
        return new_mat

# コンパイラとフラグ
CXX      := clang++-19
CXXFLAGS := @compile_flags.txt -O2 -MMD -MP

# ソースディレクトリとビルドディレクトリ
SRC_DIR := examples
BUILD_DIR := build

# 実行ファイル名
EXEC := ap.out

# ソース一覧を自動探索
SRC := $(shell find $(SRC_DIR) -name '*.cpp')

# ソースから対応するオブジェクトファイル名を作成
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

# デフォルトターゲット
all: $(EXEC)

# リンク
$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# コンパイル規則（ビルドディレクトリを自動作成）
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# クリーン
clean:
	rm -rf $(BUILD_DIR) $(EXEC)

# 依存関係ファイルを include
-include $(DEP)
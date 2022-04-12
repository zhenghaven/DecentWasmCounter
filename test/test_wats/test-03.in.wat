(module
  (import "env" "decent_wasm_test_log" (func $log (param i32)))
  (import "env" "decent_wasm_counter_exceed" (func $ctr_exceed (param i32) (param i32) (result i32)))

  (func $main_func
    (local $i i32)

    local.get $i
    i32.const 1
    i32.add ;; w = 1
    local.set $i
    local.get $i
    call $log ;; w = 10
    ;; total_w = 11

    block $blk_1
      loop $loop_1
        block $blk_2
          local.get $i
          i32.const 1
          i32.le_u ;; w = 1
          ;; jump into loop
          ;; total_w = 1
          br_if $loop_1

          local.get $i
          i32.const 1
          i32.add ;; w = 1
          local.set $i
          local.get $i
          i32.const 1
          i32.eq ;; w = 1
          ;; jump back to loop
          ;; total_w = 2
          br_if $blk_2

          local.get $i
          i32.const 1
          i32.add ;; w = 1
          local.set $i
          local.get $i
          i32.const 1
          i32.eq ;; w = 1
          ;; jump out of loop
          ;; total_w = 2
          br_if $blk_1
        end
      end
    end

    block $blk_1
      loop $loop_1
        block $blk_2
          local.get $i
          i32.const 1
          i32.le_u ;; w = 1
          ;; jump into loop
          ;; total_w = 1
          br_if $loop_1

          local.get $i
          i32.const 1
          i32.add ;; w = 1
          local.set $i
          local.get $i
          i32.const 1
          i32.eq ;; w = 1
          ;; jump back to loop
          ;; total_w = 2
          br_if $blk_2

          local.get $i
          i32.const 1
          i32.add ;; w = 1
          local.set $i
          local.get $i
          i32.const 1
          i32.eq ;; w = 1
          ;; jump out of loop
          ;; total_w = 2
          br_if $blk_1
        end
        local.get $i
        i32.const 1
        i32.add ;; w = 1
        local.set $i
        ;; total_w = 1
      end
      local.get $i
      call $log ;; w = 10
      ;; total_w = 10
    end

    local.get $i
    i32.const 1
    i32.add ;; w = 1
    local.set $i
    local.get $i
    call $log ;; w = 10
    ;; total_w = 11
  )

  (start 2)
)

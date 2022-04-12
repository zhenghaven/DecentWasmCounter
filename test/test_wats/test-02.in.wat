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

    block
      block
        local.get $i
        i32.const 1
        i32.eq ;; w = 1
        br_if 1
        local.get $i
        i32.const 1
        i32.add ;; w = 1
        local.set $i
        local.get $i
        call $log ;; w = 10
      end
    end

    local.get $i
    i32.const 1
    i32.add ;; w = 1
    local.set $i
    local.get $i
    call $log ;; w = 10
  )

  (func $empty_func)

  (start 2)
)

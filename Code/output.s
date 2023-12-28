.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text

read:
	li $v0, 4
	la $a0, _prompt
	syscall
	li $v0, 5
	syscall
	jr $ra

write:
	li $v0, 1
	syscall
	li $v0, 4
	la $a0, _ret
	syscall
	move $v0, $0
	jr $ra


main:
	subu $sp, $sp, 4
	sw $ra, 0($sp)
	subu $sp, $sp, 4
	sw $fp, 0($sp)
	move $fp, $sp
	addi $sp, $sp, -48
	li $t1, 4
	move $t0, $t1
	sw $t0, -12($fp)
	addi $t0, $fp, -8
	sw $t0, -16($fp)
	li $t1, 1
	move $t0, $t1
	sw $t0, -20($fp)
	lw $t1, -20($fp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0, -20($fp)
	lw $t1, -16($fp)
	lw $t2, -20($fp)
	add $t0, $t1, $t2
	sw $t0, -24($fp)
	lw $t1, -24($fp)
	move $t0, $t1
	sw $t0, -28($fp)
	lw $t1, -12($fp)
	lw $t0, -28($fp)
	sw $t1, 0($t0)
	addi $t0, $fp, -8
	sw $t0, -32($fp)
	li $t1, 1
	move $t0, $t1
	sw $t0, -36($fp)
	lw $t1, -36($fp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0, -36($fp)
	lw $t1, -32($fp)
	lw $t2, -36($fp)
	add $t0, $t1, $t2
	sw $t0, -40($fp)
	lw $t1, -40($fp)
	lw $t0, 0($t1)
	sw $t0, -44($fp)
	lw $a0, -44($fp)
	jal write
	li $t1, 0
	move $t0, $t1
	sw $t0, -48($fp)
	lw $v0, -48($fp)
	move $sp, $fp
	lw $fp, 0($sp)
	addi $sp, $sp, 4
	lw $ra, 0($sp)
	addi $sp, $sp, 4
	jr $ra

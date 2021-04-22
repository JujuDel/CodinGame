(ns Solution
  (:require [clojure.string :as str])
  (:gen-class))

; Auto-generated code below aims at helping you parse
; the standard input according to the problem statement.

(defn output [msg] (println msg) (flush))
(defn debug [msg] (binding [*out* *err*] (println msg) (flush)))

(defn -main [& args]
  (let [[L C N] (map #(Integer/parseInt %) (str/split (read-line) #" "))]
    (def MEM_id (vec (replicate N 0)))
    (loop [i 0]
      (when (< i N)
        (let [Pi (Integer/parseInt (read-line))]
          (def MEM_id (assoc MEM_id i Pi))
        )
        (recur (inc i))
      )
    )
    (def umap {})

    (def idx (atom 0))
    (def rep (atom (long 0)))
    (def res (atom (long 0)))
    (def c (atom (long 0)))
    (def l (atom (long 0)))
    (def n (atom (long 0)))

    ; For all the rides within the day
    ; Stop if all the rides are done or if we achieved a repeated position
    (while (< @c C)
    (do
      (if (contains? umap @idx)
      (do
        (def pair (get umap @idx))
        (def longRes (long (unchecked-multiply
            (long (- @res (get pair :res)))
            (long (quot (long (- C @c)) (- @c (get pair :c))))
        )))
        (swap! res (partial + longRes))
        (def c2 (atom (- C (mod (- C @c) (- @c (get pair :c))))))

        (while (<  @c2 C)
        (do
          (reset! l 0)
          (reset! n 0)
          (while (and (< @n N) (<= (+ @l (get MEM_id @idx)) L))
          (do
            (swap! l (partial + (get MEM_id @idx)))
            (swap! n inc)
            (swap! idx inc)
            (if (= @idx N) (do (reset! idx 0)))
          ))
          (swap! res (partial + @l))
          (swap! c2 inc)
        ))

        (swap! c (partial + C))
      )
      (do
        (def umap (assoc umap (identity @idx) {:res (identity @res), :c (identity @c)}))
        (reset! l 0)
        (reset! n 0)
        (while (and (< @n N) (<= (+ @l (get MEM_id @idx)) L))
        (do
          (swap! l (partial + (get MEM_id @idx)))
          (swap! n inc)
          (swap! idx inc)
          (if (= @idx N) (do (reset! idx 0)))
        ))
        (swap! res (partial + @l))
        (swap! c inc)
      ))
    ))

    ; Write answer to stdout
    (output (+ @res @rep))
  )
)